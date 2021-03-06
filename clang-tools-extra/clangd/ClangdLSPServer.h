//===--- ClangdLSPServer.h - LSP server --------------------------*- C++-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANGD_CLANGDLSPSERVER_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANGD_CLANGDLSPSERVER_H

#include "ClangdServer.h"
#include "GlobalCompilationDatabase.h"
#include "Path.h"
#include "Protocol.h"
#include "clang/Tooling/Core/Replacement.h"
#include "llvm/ADT/Optional.h"

namespace clang {
namespace clangd {

class JSONOutput;

/// This class provides implementation of an LSP server, glueing the JSON
/// dispatch and ClangdServer together.
class ClangdLSPServer {
public:
  ClangdLSPServer(JSONOutput &Out, unsigned AsyncThreadsCount,
                  llvm::Optional<StringRef> ResourceDir);

  /// Run LSP server loop, receiving input for it from \p In. \p In must be
  /// opened in binary mode. Output will be written using Out variable passed to
  /// class constructor. This method must not be executed more than once for
  /// each instance of ClangdLSPServer.
  void run(std::istream &In);

private:
  class LSPProtocolCallbacks;
  class LSPDiagnosticsConsumer : public DiagnosticsConsumer {
  public:
    LSPDiagnosticsConsumer(ClangdLSPServer &Server);

    virtual void
    onDiagnosticsReady(PathRef File,
                       Tagged<std::vector<DiagWithFixIts>> Diagnostics);

  private:
    ClangdLSPServer &Server;
  };

  std::vector<clang::tooling::Replacement>
  getFixIts(StringRef File, const clangd::Diagnostic &D);

  /// Function that will be called on a separate thread when diagnostics are
  /// ready. Sends the Dianostics to LSP client via Out.writeMessage and caches
  /// corresponding fixits in the FixItsMap.
  void consumeDiagnostics(PathRef File,
                          std::vector<DiagWithFixIts> Diagnostics);

  JSONOutput &Out;
  /// Used to indicate that the 'shutdown' request was received from the
  /// Language Server client.
  /// It's used to break out of the LSP parsing loop.
  bool IsDone = false;

  std::mutex FixItsMutex;
  typedef std::map<clangd::Diagnostic, std::vector<clang::tooling::Replacement>>
      DiagnosticToReplacementMap;
  /// Caches FixIts per file and diagnostics
  llvm::StringMap<DiagnosticToReplacementMap> FixItsMap;

  // Various ClangdServer parameters go here. It's important they're created
  // before ClangdServer.
  DirectoryBasedGlobalCompilationDatabase CDB;
  LSPDiagnosticsConsumer DiagConsumer;
  RealFileSystemProvider FSProvider;

  // Server must be the last member of the class to allow its destructor to exit
  // the worker thread that may otherwise run an async callback on partially
  // destructed instance of ClangdLSPServer.
  ClangdServer Server;
};

} // namespace clangd
} // namespace clang

#endif
