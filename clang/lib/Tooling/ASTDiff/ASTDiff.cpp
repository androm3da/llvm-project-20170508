//===- ASTDiff.cpp - AST differencing implementation-----------*- C++ -*- -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains definitons for the AST differencing interface.
//
//===----------------------------------------------------------------------===//

#include "clang/Tooling/ASTDiff/ASTDiff.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/PriorityQueue.h"

#include <limits>
#include <memory>
#include <unordered_set>

using namespace llvm;
using namespace clang;

namespace clang {
namespace diff {

namespace {
/// Maps nodes of the left tree to ones on the right, and vice versa.
class Mapping {
public:
  Mapping() = default;
  Mapping(Mapping &&Other) = default;
  Mapping &operator=(Mapping &&Other) = default;
  Mapping(int Size1, int Size2) {
    // Maximum possible size after patching one tree.
    int Size = Size1 + Size2;
    SrcToDst = llvm::make_unique<SmallVector<NodeId, 2>[]>(Size);
    DstToSrc = llvm::make_unique<SmallVector<NodeId, 2>[]>(Size);
  }

  void link(NodeId Src, NodeId Dst) {
    SrcToDst[Src].push_back(Dst);
    DstToSrc[Dst].push_back(Src);
  }

  NodeId getDst(NodeId Src) const {
    if (hasSrc(Src))
      return SrcToDst[Src][0];
    return NodeId();
  }
  NodeId getSrc(NodeId Dst) const {
    if (hasDst(Dst))
      return DstToSrc[Dst][0];
    return NodeId();
  }
  const SmallVector<NodeId, 2> &getAllDsts(NodeId Src) const {
    return SrcToDst[Src];
  }
  const SmallVector<NodeId, 2> &getAllSrcs(NodeId Dst) const {
    return DstToSrc[Dst];
  }
  bool hasSrc(NodeId Src) const { return !SrcToDst[Src].empty(); }
  bool hasDst(NodeId Dst) const { return !DstToSrc[Dst].empty(); }
  bool hasSrcDst(NodeId Src, NodeId Dst) const {
    for (NodeId DstId : SrcToDst[Src])
      if (DstId == Dst)
        return true;
    for (NodeId SrcId : DstToSrc[Dst])
      if (SrcId == Src)
        return true;
    return false;
  }

private:
  std::unique_ptr<SmallVector<NodeId, 2>[]> SrcToDst, DstToSrc;
};
} // end anonymous namespace

class ASTDiff::Impl {
public:
  SyntaxTree::Impl &T1, &T2;
  bool IsMappingDone = false;
  Mapping TheMapping;

  Impl(SyntaxTree::Impl &T1, SyntaxTree::Impl &T2,
       const ComparisonOptions &Options)
      : T1(T1), T2(T2), Options(Options) {}

  /// Matches nodes one-by-one based on their similarity.
  void computeMapping();

  std::vector<Match> getMatches(Mapping &M);

  /// Finds an edit script that converts T1 to T2.
  std::vector<Change> computeChanges(Mapping &M);

  void printChangeImpl(raw_ostream &OS, const Change &Chg) const;
  void printMatchImpl(raw_ostream &OS, const Match &M) const;

  // Returns a mapping of identical subtrees.
  Mapping matchTopDown() const;

private:
  // Returns true if the two subtrees are identical.
  bool identical(NodeId Id1, NodeId Id2) const;

  bool canBeAddedToMapping(const Mapping &M, NodeId Id1, NodeId Id2) const;

  // Returns false if the nodes must not be mached.
  bool isMatchingPossible(NodeId Id1, NodeId Id2) const;

  // Uses an optimal albeit slow algorithm to compute a mapping between two
  // subtrees, but only if both have fewer nodes than MaxSize.
  void addOptimalMapping(Mapping &M, NodeId Id1, NodeId Id2) const;

  // Computes the ratio of common descendants between the two nodes.
  // Descendants are only considered to be equal when they are mapped in M.
  double getSimilarity(const Mapping &M, NodeId Id1, NodeId Id2) const;

  // Returns the node that has the highest degree of similarity.
  NodeId findCandidate(const Mapping &M, NodeId Id1) const;

  // Tries to match any yet unmapped nodes, in a bottom-up fashion.
  void matchBottomUp(Mapping &M) const;

  const ComparisonOptions &Options;

  friend class ZhangShashaMatcher;
};

/// Represents the AST of a TranslationUnit.
class SyntaxTree::Impl {
public:
  /// Constructs a tree from the entire translation unit.
  Impl(SyntaxTree *Parent, const ASTContext &AST);
  /// Constructs a tree from an AST node.
  Impl(SyntaxTree *Parent, Decl *N, const ASTContext &AST);
  Impl(SyntaxTree *Parent, Stmt *N, const ASTContext &AST);
  template <class T>
  Impl(SyntaxTree *Parent,
       typename std::enable_if<std::is_base_of<Stmt, T>::value, T>::type *Node,
       const ASTContext &AST)
      : Impl(Parent, dyn_cast<Stmt>(Node), AST) {}
  template <class T>
  Impl(SyntaxTree *Parent,
       typename std::enable_if<std::is_base_of<Decl, T>::value, T>::type *Node,
       const ASTContext &AST)
      : Impl(Parent, dyn_cast<Decl>(Node), AST) {}

  SyntaxTree *Parent;
  const ASTContext &AST;
  std::vector<NodeId> Leaves;
  // Maps preorder indices to postorder ones.
  std::vector<int> PostorderIds;

  int getSize() const { return Nodes.size(); }
  NodeId getRootId() const { return 0; }

  const Node &getNode(NodeId Id) const { return Nodes[Id]; }
  Node &getMutableNode(NodeId Id) { return Nodes[Id]; }
  bool isValidNodeId(NodeId Id) const { return Id >= 0 && Id < getSize(); }
  void addNode(Node &N) { Nodes.push_back(N); }
  int getNumberOfDescendants(NodeId Id) const;
  bool isInSubtree(NodeId Id, NodeId SubtreeRoot) const;

  std::string getNodeValue(NodeId Id) const;
  std::string getNodeValue(const DynTypedNode &DTN) const;
  /// Prints the node as "<type>[: <value>](<postorder-id)"
  void printNode(NodeId Id) const { printNode(llvm::outs(), Id); }
  void printNode(raw_ostream &OS, NodeId Id) const;

  void printTree() const;
  void printTree(NodeId Root) const;
  void printTree(raw_ostream &OS, NodeId Root) const;

  void printAsJsonImpl(raw_ostream &OS) const;
  void printNodeAsJson(raw_ostream &OS, NodeId Id) const;

private:
  /// Nodes in preorder.
  std::vector<Node> Nodes;

  void initTree();
  void setLeftMostDescendants();
};

template <class T>
static bool isNodeExcluded(const SourceManager &SrcMgr, T *N) {
  if (!N)
    return true;
  SourceLocation SLoc = N->getLocStart();
  return SLoc.isValid() && SrcMgr.isInSystemHeader(SLoc);
}

namespace {
/// Counts the number of nodes that will be compared.
struct NodeCountVisitor : public RecursiveASTVisitor<NodeCountVisitor> {
  int Count = 0;
  const SyntaxTree::Impl &Tree;
  NodeCountVisitor(const SyntaxTree::Impl &Tree) : Tree(Tree) {}
  bool TraverseDecl(Decl *D) {
    if (isNodeExcluded(Tree.AST.getSourceManager(), D))
      return true;
    ++Count;
    RecursiveASTVisitor<NodeCountVisitor>::TraverseDecl(D);
    return true;
  }
  bool TraverseStmt(Stmt *S) {
    if (isNodeExcluded(Tree.AST.getSourceManager(), S))
      return true;
    ++Count;
    RecursiveASTVisitor<NodeCountVisitor>::TraverseStmt(S);
    return true;
  }
  bool TraverseType(QualType T) { return true; }
};
} // end anonymous namespace

namespace {
// Sets Height, Parent and Children for each node.
struct PreorderVisitor : public RecursiveASTVisitor<PreorderVisitor> {
  int Id = 0, Depth = 0;
  NodeId Parent;
  SyntaxTree::Impl &Tree;

  PreorderVisitor(SyntaxTree::Impl &Tree) : Tree(Tree) {}

  template <class T> std::tuple<NodeId, NodeId> PreTraverse(T *ASTNode) {
    NodeId MyId = Id;
    Node &N = Tree.getMutableNode(MyId);
    N.Parent = Parent;
    N.Depth = Depth;
    N.ASTNode = DynTypedNode::create(*ASTNode);
    assert(!N.ASTNode.getNodeKind().isNone() &&
           "Expected nodes to have a valid kind.");
    if (Parent.isValid()) {
      Node &P = Tree.getMutableNode(Parent);
      P.Children.push_back(MyId);
    }
    Parent = MyId;
    ++Id;
    ++Depth;
    return std::make_tuple(MyId, Tree.getNode(MyId).Parent);
  }
  void PostTraverse(std::tuple<NodeId, NodeId> State) {
    NodeId MyId, PreviousParent;
    std::tie(MyId, PreviousParent) = State;
    assert(MyId.isValid() && "Expecting to only traverse valid nodes.");
    Parent = PreviousParent;
    --Depth;
    Node &N = Tree.getMutableNode(MyId);
    N.RightMostDescendant = Id - 1;
    assert(N.RightMostDescendant >= 0 &&
           N.RightMostDescendant < Tree.getSize() &&
           "Rightmost descendant must be a valid tree node.");
    if (N.isLeaf())
      Tree.Leaves.push_back(MyId);
    N.Height = 1;
    for (NodeId Child : N.Children)
      N.Height = std::max(N.Height, 1 + Tree.getNode(Child).Height);
  }
  bool TraverseDecl(Decl *D) {
    if (isNodeExcluded(Tree.AST.getSourceManager(), D))
      return true;
    auto SavedState = PreTraverse(D);
    RecursiveASTVisitor<PreorderVisitor>::TraverseDecl(D);
    PostTraverse(SavedState);
    return true;
  }
  bool TraverseStmt(Stmt *S) {
    if (isNodeExcluded(Tree.AST.getSourceManager(), S))
      return true;
    auto SavedState = PreTraverse(S);
    RecursiveASTVisitor<PreorderVisitor>::TraverseStmt(S);
    PostTraverse(SavedState);
    return true;
  }
  bool TraverseType(QualType T) { return true; }
};
} // end anonymous namespace

SyntaxTree::Impl::Impl(SyntaxTree *Parent, const ASTContext &AST)
    : Impl(Parent, AST.getTranslationUnitDecl(), AST) {}

SyntaxTree::Impl::Impl(SyntaxTree *Parent, Decl *N, const ASTContext &AST)
    : Parent(Parent), AST(AST) {
  NodeCountVisitor NodeCounter(*this);
  NodeCounter.TraverseDecl(N);
  Nodes.resize(NodeCounter.Count);
  PreorderVisitor PreorderWalker(*this);
  PreorderWalker.TraverseDecl(N);
  initTree();
}

SyntaxTree::Impl::Impl(SyntaxTree *Parent, Stmt *N, const ASTContext &AST)
    : Parent(Parent), AST(AST) {
  NodeCountVisitor NodeCounter(*this);
  NodeCounter.TraverseStmt(N);
  Nodes.resize(NodeCounter.Count);
  PreorderVisitor PreorderWalker(*this);
  PreorderWalker.TraverseStmt(N);
  initTree();
}

void SyntaxTree::Impl::initTree() {
  setLeftMostDescendants();
  int PostorderId = 0;
  PostorderIds.resize(getSize());
  std::function<void(NodeId)> PostorderTraverse = [&](NodeId Id) {
    for (NodeId Child : getNode(Id).Children)
      PostorderTraverse(Child);
    PostorderIds[Id] = PostorderId;
    ++PostorderId;
  };
  PostorderTraverse(getRootId());
}

void SyntaxTree::Impl::setLeftMostDescendants() {
  for (NodeId Leaf : Leaves) {
    getMutableNode(Leaf).LeftMostDescendant = Leaf;
    NodeId Parent, Cur = Leaf;
    while ((Parent = getNode(Cur).Parent).isValid() &&
           getNode(Parent).Children[0] == Cur) {
      Cur = Parent;
      getMutableNode(Cur).LeftMostDescendant = Leaf;
    }
  }
}

static std::vector<NodeId> getSubtreePostorder(const SyntaxTree::Impl &Tree,
                                               NodeId Root) {
  std::vector<NodeId> Postorder;
  std::function<void(NodeId)> Traverse = [&](NodeId Id) {
    const Node &N = Tree.getNode(Id);
    for (NodeId Child : N.Children)
      Traverse(Child);
    Postorder.push_back(Id);
  };
  Traverse(Root);
  return Postorder;
}

static std::vector<NodeId> getSubtreeBfs(const SyntaxTree::Impl &Tree,
                                         NodeId Root) {
  std::vector<NodeId> Ids;
  size_t Expanded = 0;
  Ids.push_back(Root);
  while (Expanded < Ids.size())
    for (NodeId Child : Tree.getNode(Ids[Expanded++]).Children)
      Ids.push_back(Child);
  return Ids;
}

int SyntaxTree::Impl::getNumberOfDescendants(NodeId Id) const {
  return getNode(Id).RightMostDescendant - Id + 1;
}

bool SyntaxTree::Impl::isInSubtree(NodeId Id, NodeId SubtreeRoot) const {
  return Id >= SubtreeRoot && Id <= getNode(SubtreeRoot).RightMostDescendant;
}

std::string SyntaxTree::Impl::getNodeValue(NodeId Id) const {
  return getNodeValue(getNode(Id).ASTNode);
}

std::string SyntaxTree::Impl::getNodeValue(const DynTypedNode &DTN) const {
  if (auto *X = DTN.get<BinaryOperator>())
    return X->getOpcodeStr();
  if (auto *X = DTN.get<AccessSpecDecl>()) {
    CharSourceRange Range(X->getSourceRange(), false);
    return Lexer::getSourceText(Range, AST.getSourceManager(),
                                AST.getLangOpts());
  }
  if (auto *X = DTN.get<IntegerLiteral>()) {
    SmallString<256> Str;
    X->getValue().toString(Str, /*Radix=*/10, /*Signed=*/false);
    return Str.str();
  }
  if (auto *X = DTN.get<StringLiteral>())
    return X->getString();
  if (auto *X = DTN.get<ValueDecl>())
    return X->getNameAsString() + "(" + X->getType().getAsString() + ")";
  if (DTN.get<DeclStmt>() || DTN.get<TranslationUnitDecl>())
    return "";
  std::string Value;
  if (auto *X = DTN.get<DeclRefExpr>()) {
    if (X->hasQualifier()) {
      llvm::raw_string_ostream OS(Value);
      PrintingPolicy PP(AST.getLangOpts());
      X->getQualifier()->print(OS, PP);
    }
    Value += X->getDecl()->getNameAsString();
    return Value;
  }
  if (auto *X = DTN.get<NamedDecl>())
    Value += X->getNameAsString() + ";";
  if (auto *X = DTN.get<TypedefNameDecl>())
    return Value + X->getUnderlyingType().getAsString() + ";";
  if (DTN.get<NamespaceDecl>())
    return Value;
  if (auto *X = DTN.get<TypeDecl>())
    if (X->getTypeForDecl())
      Value +=
          X->getTypeForDecl()->getCanonicalTypeInternal().getAsString() + ";";
  if (DTN.get<Decl>())
    return Value;
  if (DTN.get<Stmt>())
    return "";
  llvm_unreachable("Fatal: unhandled AST node.\n");
}

void SyntaxTree::Impl::printTree() const { printTree(getRootId()); }
void SyntaxTree::Impl::printTree(NodeId Root) const {
  printTree(llvm::outs(), Root);
}

void SyntaxTree::Impl::printTree(raw_ostream &OS, NodeId Root) const {
  const Node &N = getNode(Root);
  for (int I = 0; I < N.Depth; ++I)
    OS << " ";
  printNode(OS, Root);
  OS << "\n";
  for (NodeId Child : N.Children)
    printTree(OS, Child);
}

void SyntaxTree::Impl::printNode(raw_ostream &OS, NodeId Id) const {
  if (Id.isInvalid()) {
    OS << "None";
    return;
  }
  OS << getNode(Id).getTypeLabel();
  if (getNodeValue(Id) != "")
    OS << ": " << getNodeValue(Id);
  OS << "(" << PostorderIds[Id] << ")";
}

void SyntaxTree::Impl::printNodeAsJson(raw_ostream &OS, NodeId Id) const {
  auto N = getNode(Id);
  OS << R"({"type":")" << N.getTypeLabel() << R"(")";
  if (getNodeValue(Id) != "")
    OS << R"(,"value":")" << getNodeValue(Id) << R"(")";
  OS << R"(,"children":[)";
  if (N.Children.size() > 0) {
    printNodeAsJson(OS, N.Children[0]);
    for (size_t I = 1, E = N.Children.size(); I < E; ++I) {
      OS << ",";
      printNodeAsJson(OS, N.Children[I]);
    }
  }
  OS << "]}";
}

void SyntaxTree::Impl::printAsJsonImpl(raw_ostream &OS) const {
  OS << R"({"root":)";
  printNodeAsJson(OS, getRootId());
  OS << "}\n";
}

/// Identifies a node in a subtree by its postorder offset, starting at 1.
struct SNodeId {
  int Id = 0;

  explicit SNodeId(int Id) : Id(Id) {}
  explicit SNodeId() = default;

  operator int() const { return Id; }
  SNodeId &operator++() { return ++Id, *this; }
  SNodeId &operator--() { return --Id, *this; }
  SNodeId operator+(int Other) const { return SNodeId(Id + Other); }
};

class Subtree {
private:
  /// The parent tree.
  const SyntaxTree::Impl &Tree;
  /// Maps SNodeIds to original ids.
  std::vector<NodeId> RootIds;
  /// Maps subtree nodes to their leftmost descendants wtihin the subtree.
  std::vector<SNodeId> LeftMostDescendants;

public:
  std::vector<SNodeId> KeyRoots;

  Subtree(const SyntaxTree::Impl &Tree, NodeId SubtreeRoot) : Tree(Tree) {
    RootIds = getSubtreePostorder(Tree, SubtreeRoot);
    int NumLeaves = setLeftMostDescendants();
    computeKeyRoots(NumLeaves);
  }
  int getSize() const { return RootIds.size(); }
  NodeId getIdInRoot(SNodeId Id) const {
    assert(Id > 0 && Id <= getSize() && "Invalid subtree node index.");
    return RootIds[Id - 1];
  }
  const Node &getNode(SNodeId Id) const {
    return Tree.getNode(getIdInRoot(Id));
  }
  SNodeId getLeftMostDescendant(SNodeId Id) const {
    assert(Id > 0 && Id <= getSize() && "Invalid subtree node index.");
    return LeftMostDescendants[Id - 1];
  }
  /// Returns the postorder index of the leftmost descendant in the subtree.
  NodeId getPostorderOffset() const {
    return Tree.PostorderIds[getIdInRoot(SNodeId(1))];
  }
  std::string getNodeValue(SNodeId Id) const {
    return Tree.getNodeValue(getIdInRoot(Id));
  }

private:
  /// Returns the number of leafs in the subtree.
  int setLeftMostDescendants() {
    int NumLeaves = 0;
    LeftMostDescendants.resize(getSize());
    for (int I = 0; I < getSize(); ++I) {
      SNodeId SI(I + 1);
      const Node &N = getNode(SI);
      NumLeaves += N.isLeaf();
      assert(I == Tree.PostorderIds[getIdInRoot(SI)] - getPostorderOffset() &&
             "Postorder traversal in subtree should correspond to traversal in "
             "the root tree by a constant offset.");
      LeftMostDescendants[I] = SNodeId(Tree.PostorderIds[N.LeftMostDescendant] -
                                       getPostorderOffset());
    }
    return NumLeaves;
  }
  void computeKeyRoots(int Leaves) {
    KeyRoots.resize(Leaves);
    std::unordered_set<int> Visited;
    int K = Leaves - 1;
    for (SNodeId I(getSize()); I > 0; --I) {
      SNodeId LeftDesc = getLeftMostDescendant(I);
      if (Visited.count(LeftDesc))
        continue;
      assert(K >= 0 && "K should be non-negative");
      KeyRoots[K] = I;
      Visited.insert(LeftDesc);
      --K;
    }
  }
};

/// Implementation of Zhang and Shasha's Algorithm for tree edit distance.
/// Computes an optimal mapping between two trees using only insertion,
/// deletion and update as edit actions (similar to the Levenshtein distance).
class ZhangShashaMatcher {
  const ASTDiff::Impl &DiffImpl;
  Subtree S1;
  Subtree S2;
  std::unique_ptr<std::unique_ptr<double[]>[]> TreeDist, ForestDist;

public:
  ZhangShashaMatcher(const ASTDiff::Impl &DiffImpl, const SyntaxTree::Impl &T1,
                     const SyntaxTree::Impl &T2, NodeId Id1, NodeId Id2)
      : DiffImpl(DiffImpl), S1(T1, Id1), S2(T2, Id2) {
    TreeDist = llvm::make_unique<std::unique_ptr<double[]>[]>(
        size_t(S1.getSize()) + 1);
    ForestDist = llvm::make_unique<std::unique_ptr<double[]>[]>(
        size_t(S1.getSize()) + 1);
    for (int I = 0, E = S1.getSize() + 1; I < E; ++I) {
      TreeDist[I] = llvm::make_unique<double[]>(size_t(S2.getSize()) + 1);
      ForestDist[I] = llvm::make_unique<double[]>(size_t(S2.getSize()) + 1);
    }
  }

  std::vector<std::pair<NodeId, NodeId>> getMatchingNodes() {
    std::vector<std::pair<NodeId, NodeId>> Matches;
    std::vector<std::pair<SNodeId, SNodeId>> TreePairs;

    computeTreeDist();

    bool RootNodePair = true;

    TreePairs.emplace_back(SNodeId(S1.getSize()), SNodeId(S2.getSize()));

    while (!TreePairs.empty()) {
      SNodeId LastRow, LastCol, FirstRow, FirstCol, Row, Col;
      std::tie(LastRow, LastCol) = TreePairs.back();
      TreePairs.pop_back();

      if (!RootNodePair) {
        computeForestDist(LastRow, LastCol);
      }

      RootNodePair = false;

      FirstRow = S1.getLeftMostDescendant(LastRow);
      FirstCol = S2.getLeftMostDescendant(LastCol);

      Row = LastRow;
      Col = LastCol;

      while (Row > FirstRow || Col > FirstCol) {
        if (Row > FirstRow &&
            ForestDist[Row - 1][Col] + 1 == ForestDist[Row][Col]) {
          --Row;
        } else if (Col > FirstCol &&
                   ForestDist[Row][Col - 1] + 1 == ForestDist[Row][Col]) {
          --Col;
        } else {
          SNodeId LMD1 = S1.getLeftMostDescendant(Row);
          SNodeId LMD2 = S2.getLeftMostDescendant(Col);
          if (LMD1 == S1.getLeftMostDescendant(LastRow) &&
              LMD2 == S2.getLeftMostDescendant(LastCol)) {
            NodeId Id1 = S1.getIdInRoot(Row);
            NodeId Id2 = S2.getIdInRoot(Col);
            assert(DiffImpl.isMatchingPossible(Id1, Id2) &&
                   "These nodes must not be matched.");
            Matches.emplace_back(Id1, Id2);
            --Row;
            --Col;
          } else {
            TreePairs.emplace_back(Row, Col);
            Row = LMD1;
            Col = LMD2;
          }
        }
      }
    }
    return Matches;
  }

private:
  /// We use a simple cost model for edit actions, which seems good enough.
  /// Simple cost model for edit actions. This seems to make the matching
  /// algorithm perform reasonably well.
  /// The values range between 0 and 1, or infinity if this edit action should
  /// always be avoided.
  static constexpr double DeletionCost = 1;
  static constexpr double InsertionCost = 1;

  double getUpdateCost(SNodeId Id1, SNodeId Id2) {
    if (!DiffImpl.isMatchingPossible(S1.getIdInRoot(Id1), S2.getIdInRoot(Id2)))
      return std::numeric_limits<double>::max();
    return S1.getNodeValue(Id1) != S2.getNodeValue(Id2);
  }

  void computeTreeDist() {
    for (SNodeId Id1 : S1.KeyRoots)
      for (SNodeId Id2 : S2.KeyRoots)
        computeForestDist(Id1, Id2);
  }

  void computeForestDist(SNodeId Id1, SNodeId Id2) {
    assert(Id1 > 0 && Id2 > 0 && "Expecting offsets greater than 0.");
    SNodeId LMD1 = S1.getLeftMostDescendant(Id1);
    SNodeId LMD2 = S2.getLeftMostDescendant(Id2);

    ForestDist[LMD1][LMD2] = 0;
    for (SNodeId D1 = LMD1 + 1; D1 <= Id1; ++D1) {
      ForestDist[D1][LMD2] = ForestDist[D1 - 1][LMD2] + DeletionCost;
      for (SNodeId D2 = LMD2 + 1; D2 <= Id2; ++D2) {
        ForestDist[LMD1][D2] = ForestDist[LMD1][D2 - 1] + InsertionCost;
        SNodeId DLMD1 = S1.getLeftMostDescendant(D1);
        SNodeId DLMD2 = S2.getLeftMostDescendant(D2);
        if (DLMD1 == LMD1 && DLMD2 == LMD2) {
          double UpdateCost = getUpdateCost(D1, D2);
          ForestDist[D1][D2] =
              std::min({ForestDist[D1 - 1][D2] + DeletionCost,
                        ForestDist[D1][D2 - 1] + InsertionCost,
                        ForestDist[D1 - 1][D2 - 1] + UpdateCost});
          TreeDist[D1][D2] = ForestDist[D1][D2];
        } else {
          ForestDist[D1][D2] =
              std::min({ForestDist[D1 - 1][D2] + DeletionCost,
                        ForestDist[D1][D2 - 1] + InsertionCost,
                        ForestDist[DLMD1][DLMD2] + TreeDist[D1][D2]});
        }
      }
    }
  }
};

namespace {
// Compares nodes by their depth.
struct HeightLess {
  const SyntaxTree::Impl &Tree;
  HeightLess(const SyntaxTree::Impl &Tree) : Tree(Tree) {}
  bool operator()(NodeId Id1, NodeId Id2) const {
    return Tree.getNode(Id1).Height < Tree.getNode(Id2).Height;
  }
};
} // end anonymous namespace

namespace {
// Priority queue for nodes, sorted descendingly by their height.
class PriorityList {
  const SyntaxTree::Impl &Tree;
  HeightLess Cmp;
  std::vector<NodeId> Container;
  PriorityQueue<NodeId, std::vector<NodeId>, HeightLess> List;

public:
  PriorityList(const SyntaxTree::Impl &Tree)
      : Tree(Tree), Cmp(Tree), List(Cmp, Container) {}

  void push(NodeId id) { List.push(id); }

  std::vector<NodeId> pop() {
    int Max = peekMax();
    std::vector<NodeId> Result;
    if (Max == 0)
      return Result;
    while (peekMax() == Max) {
      Result.push_back(List.top());
      List.pop();
    }
    // TODO this is here to get a stable output, not a good heuristic
    std::sort(Result.begin(), Result.end());
    return Result;
  }
  int peekMax() const {
    if (List.empty())
      return 0;
    return Tree.getNode(List.top()).Height;
  }
  void open(NodeId Id) {
    for (NodeId Child : Tree.getNode(Id).Children)
      push(Child);
  }
};
} // end anonymous namespace

bool ASTDiff::Impl::identical(NodeId Id1, NodeId Id2) const {
  const Node &N1 = T1.getNode(Id1);
  const Node &N2 = T2.getNode(Id2);
  if (N1.Children.size() != N2.Children.size() ||
      !isMatchingPossible(Id1, Id2) ||
      T1.getNodeValue(Id1) != T2.getNodeValue(Id2))
    return false;
  for (size_t Id = 0, E = N1.Children.size(); Id < E; ++Id)
    if (!identical(N1.Children[Id], N2.Children[Id]))
      return false;
  return true;
}

bool ASTDiff::Impl::canBeAddedToMapping(const Mapping &M, NodeId Id1,
                                        NodeId Id2) const {
  assert(isMatchingPossible(Id1, Id2) &&
         "Matching must be possible in the first place.");
  if (M.hasSrcDst(Id1, Id2))
    return false;
  if (Options.EnableMatchingWithUnmatchableParents)
    return true;
  const Node &N1 = T1.getNode(Id1);
  const Node &N2 = T2.getNode(Id2);
  NodeId P1 = N1.Parent;
  NodeId P2 = N2.Parent;
  // Only allow matching if parents can be matched.
  return (P1.isInvalid() && P2.isInvalid()) ||
         (P1.isValid() && P2.isValid() && isMatchingPossible(P1, P2));
}

bool ASTDiff::Impl::isMatchingPossible(NodeId Id1, NodeId Id2) const {
  return Options.isMatchingAllowed(T1.getNode(Id1).ASTNode,
                                   T2.getNode(Id2).ASTNode);
}

void ASTDiff::Impl::addOptimalMapping(Mapping &M, NodeId Id1,
                                      NodeId Id2) const {
  if (std::max(T1.getNumberOfDescendants(Id1),
               T2.getNumberOfDescendants(Id2)) >= Options.MaxSize)
    return;
  ZhangShashaMatcher Matcher(*this, T1, T2, Id1, Id2);
  std::vector<std::pair<NodeId, NodeId>> R = Matcher.getMatchingNodes();
  for (const auto Tuple : R) {
    NodeId Src = Tuple.first;
    NodeId Dst = Tuple.second;
    if (canBeAddedToMapping(M, Src, Dst))
      M.link(Src, Dst);
  }
}

double ASTDiff::Impl::getSimilarity(const Mapping &M, NodeId Id1,
                                    NodeId Id2) const {
  if (Id1.isInvalid() || Id2.isInvalid())
    return 0.0;
  int CommonDescendants = 0;
  const Node &N1 = T1.getNode(Id1);
  for (NodeId Id = Id1 + 1; Id <= N1.RightMostDescendant; ++Id)
    CommonDescendants += int(T2.isInSubtree(M.getDst(Id), Id2));
  return 2.0 * CommonDescendants /
         (T1.getNumberOfDescendants(Id1) + T2.getNumberOfDescendants(Id2));
}

NodeId ASTDiff::Impl::findCandidate(const Mapping &M, NodeId Id1) const {
  NodeId Candidate;
  double HighestSimilarity = 0.0;
  for (NodeId Id2 = 0, E = T2.getSize(); Id2 < E; ++Id2) {
    if (!isMatchingPossible(Id1, Id2))
      continue;
    if (M.hasDst(Id2))
      continue;
    double Similarity = getSimilarity(M, Id1, Id2);
    if (Similarity >= Options.MinSimilarity && Similarity > HighestSimilarity) {
      HighestSimilarity = Similarity;
      Candidate = Id2;
    }
  }
  return Candidate;
}

void ASTDiff::Impl::matchBottomUp(Mapping &M) const {
  std::vector<NodeId> Postorder = getSubtreePostorder(T1, T1.getRootId());
  for (NodeId Id1 : Postorder) {
    if (Id1 == T1.getRootId() && !M.hasSrc(T1.getRootId()) &&
        !M.hasDst(T2.getRootId())) {
      if (isMatchingPossible(T1.getRootId(), T2.getRootId())) {
        M.link(T1.getRootId(), T2.getRootId());
        addOptimalMapping(M, T1.getRootId(), T2.getRootId());
      }
      break;
    }
    const Node &N1 = T1.getNode(Id1);
    bool Matched = M.hasSrc(Id1);
    bool MatchedChildren =
        std::any_of(N1.Children.begin(), N1.Children.end(),
                    [&](NodeId Child) { return M.hasSrc(Child); });
    if (Matched || !MatchedChildren)
      continue;
    NodeId Id2 = findCandidate(M, Id1);
    if (Id2.isValid() && canBeAddedToMapping(M, Id1, Id2)) {
      M.link(Id1, Id2);
      addOptimalMapping(M, Id1, Id2);
    }
  }
}

Mapping ASTDiff::Impl::matchTopDown() const {
  PriorityList L1(T1);
  PriorityList L2(T2);

  Mapping M(T1.getSize(), T2.getSize());

  L1.push(T1.getRootId());
  L2.push(T2.getRootId());

  int Max1, Max2;
  while (std::min(Max1 = L1.peekMax(), Max2 = L2.peekMax()) >
         Options.MinHeight) {
    if (Max1 > Max2) {
      for (NodeId Id : L1.pop())
        L1.open(Id);
      continue;
    }
    if (Max2 > Max1) {
      for (NodeId Id : L2.pop())
        L2.open(Id);
      continue;
    }
    std::vector<NodeId> H1, H2;
    H1 = L1.pop();
    H2 = L2.pop();
    for (NodeId Id1 : H1) {
      for (NodeId Id2 : H2) {
        if (identical(Id1, Id2) && canBeAddedToMapping(M, Id1, Id2)) {
          for (int I = 0, E = T1.getNumberOfDescendants(Id1); I < E; ++I)
            M.link(Id1 + I, Id2 + I);
        }
      }
    }
    for (NodeId Id1 : H1) {
      if (!M.hasSrc(Id1))
        L1.open(Id1);
    }
    for (NodeId Id2 : H2) {
      if (!M.hasDst(Id2))
        L2.open(Id2);
    }
  }
  return M;
}

void ASTDiff::Impl::computeMapping() {
  if (IsMappingDone)
    return;
  TheMapping = matchTopDown();
  matchBottomUp(TheMapping);
  IsMappingDone = true;
}

std::vector<Match> ASTDiff::Impl::getMatches(Mapping &M) {
  std::vector<Match> Matches;
  for (NodeId Id1 = 0, Id2, E = T1.getSize(); Id1 < E; ++Id1)
    if ((Id2 = M.getDst(Id1)).isValid())
      Matches.push_back({Id1, Id2});
  return Matches;
}

std::vector<Change> ASTDiff::Impl::computeChanges(Mapping &M) {
  std::vector<Change> Changes;
  for (NodeId Id2 : getSubtreeBfs(T2, T2.getRootId())) {
    const Node &N2 = T2.getNode(Id2);
    NodeId Id1 = M.getSrc(Id2);
    if (Id1.isValid()) {
      assert(isMatchingPossible(Id1, Id2) && "Invalid matching.");
      if (T1.getNodeValue(Id1) != T2.getNodeValue(Id2)) {
        Changes.emplace_back(Update, Id1, Id2);
      }
      continue;
    }
    NodeId P2 = N2.Parent;
    NodeId P1 = M.getSrc(P2);
    assert(P1.isValid() &&
           "Parents must be matched for determining the change type.");
    Node &Parent1 = T1.getMutableNode(P1);
    const Node &Parent2 = T2.getNode(P2);
    auto &Siblings1 = Parent1.Children;
    const auto &Siblings2 = Parent2.Children;
    size_t Position;
    for (Position = 0; Position < Siblings2.size(); ++Position)
      if (Siblings2[Position] == Id2 || Position >= Siblings1.size())
        break;
    Changes.emplace_back(Insert, Id2, P2, Position);
    Node PatchNode;
    PatchNode.Parent = P1;
    PatchNode.LeftMostDescendant = N2.LeftMostDescendant;
    PatchNode.RightMostDescendant = N2.RightMostDescendant;
    PatchNode.Depth = N2.Depth;
    PatchNode.ASTNode = N2.ASTNode;
    // TODO update Depth if needed
    NodeId PatchNodeId = T1.getSize();
    // TODO maybe choose a different data structure for Children.
    Siblings1.insert(Siblings1.begin() + Position, PatchNodeId);
    T1.addNode(PatchNode);
    M.link(PatchNodeId, Id2);
  }
  for (NodeId Id1 = 0; Id1 < T1.getSize(); ++Id1) {
    NodeId Id2 = M.getDst(Id1);
    if (Id2.isInvalid())
      Changes.emplace_back(Delete, Id1, Id2);
  }
  return Changes;
}

void ASTDiff::Impl::printChangeImpl(raw_ostream &OS, const Change &Chg) const {
  switch (Chg.Kind) {
  case Delete:
    OS << "Delete ";
    T1.printNode(OS, Chg.Src);
    OS << "\n";
    break;
  case Update:
    OS << "Update ";
    T1.printNode(OS, Chg.Src);
    OS << " to " << T2.getNodeValue(Chg.Dst) << "\n";
    break;
  case Insert:
    OS << "Insert ";
    T2.printNode(OS, Chg.Src);
    OS << " into ";
    T2.printNode(OS, Chg.Dst);
    OS << " at " << Chg.Position << "\n";
    break;
  case Move:
    llvm_unreachable("TODO");
    break;
  };
}

void ASTDiff::Impl::printMatchImpl(raw_ostream &OS, const Match &M) const {
  OS << "Match ";
  T1.printNode(OS, M.Src);
  OS << " to ";
  T2.printNode(OS, M.Dst);
  OS << "\n";
}

ASTDiff::ASTDiff(SyntaxTree &T1, SyntaxTree &T2,
                 const ComparisonOptions &Options)
    : DiffImpl(llvm::make_unique<Impl>(*T1.TreeImpl, *T2.TreeImpl, Options)) {}

ASTDiff::~ASTDiff() = default;

SyntaxTree::SyntaxTree(const ASTContext &AST)
    : TreeImpl(llvm::make_unique<SyntaxTree::Impl>(
          this, AST.getTranslationUnitDecl(), AST)) {}

std::vector<Match> ASTDiff::getMatches() {
  DiffImpl->computeMapping();
  return DiffImpl->getMatches(DiffImpl->TheMapping);
}

std::vector<Change> ASTDiff::getChanges() {
  DiffImpl->computeMapping();
  return DiffImpl->computeChanges(DiffImpl->TheMapping);
}

void ASTDiff::printChange(raw_ostream &OS, const Change &Chg) const {
  DiffImpl->printChangeImpl(OS, Chg);
}

void ASTDiff::printMatch(raw_ostream &OS, const Match &M) const {
  DiffImpl->printMatchImpl(OS, M);
}

SyntaxTree::~SyntaxTree() = default;

void SyntaxTree::printAsJson(raw_ostream &OS) { TreeImpl->printAsJsonImpl(OS); }

std::string SyntaxTree::getNodeValue(const DynTypedNode &DTN) const {
  return TreeImpl->getNodeValue(DTN);
}

} // end namespace diff
} // end namespace clang
