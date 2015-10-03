// RUN: %clang_cc1 %s -I%S -triple=x86_64-apple-darwin10 -fstrict-vtable-pointers -disable-llvm-optzns -O2 -emit-llvm -o %t.ll
// RUN: FileCheck --check-prefix=CHECK-CTORS %s < %t.ll
// RUN: FileCheck --check-prefix=CHECK-NEW %s < %t.ll
// RUN: FileCheck --check-prefix=CHECK-DTORS %s < %t.ll
// RUN: FileCheck --check-prefix=CHECK-LINK-REQ %s < %t.ll

typedef __typeof__(sizeof(0)) size_t;
void *operator new(size_t, void*) throw();

struct NotTrivialDtor {
  ~NotTrivialDtor();
};

struct DynamicBase1 {
  NotTrivialDtor obj;
  virtual void foo();
};

struct DynamicDerived : DynamicBase1 {
  void foo();
};

struct DynamicBase2 {
  virtual void bar();
  ~DynamicBase2() {
    bar();
  }
};

struct DynamicDerivedMultiple : DynamicBase1, DynamicBase2 {
  virtual void foo();
  virtual void bar();
};

struct StaticBase {
  NotTrivialDtor obj;
  void bar();
};

struct DynamicFromStatic : StaticBase {
  virtual void bar();
};

struct DynamicFromVirtualStatic1 : virtual StaticBase {
};

struct DynamicFromVirtualStatic2 : virtual StaticBase {
};

struct DynamicFrom2Virtuals :
            DynamicFromVirtualStatic1,
            DynamicFromVirtualStatic2 {
};

// CHECK-NEW-LABEL: define void @_Z12LocalObjectsv()
// CHECK-NEW-NOT: @llvm.invariant.group.barrier(
// CHECK-NEW-LABEL: }
void LocalObjects() {
  DynamicBase1 DB;
  DB.foo();
  DynamicDerived DD;
  DD.foo();

  DynamicBase2 DB2;
  DB2.bar();

  StaticBase SB;
  SB.bar();

  DynamicDerivedMultiple DDM;
  DDM.foo();
  DDM.bar();

  DynamicFromStatic DFS;
  DFS.bar();
  DynamicFromVirtualStatic1 DFVS1;
  DFVS1.bar();
  DynamicFrom2Virtuals DF2V;
  DF2V.bar();
}

struct DynamicFromVirtualStatic1;
// CHECK-CTORS-LABEL: define linkonce_odr void @_ZN25DynamicFromVirtualStatic1C1Ev
// CHECK-CTORS-NOT: @llvm.invariant.group.barrier(
// CHECK-CTORS-LABEL: }

struct DynamicFrom2Virtuals;
// CHECK-CTORS-LABEL: define linkonce_odr void @_ZN20DynamicFrom2VirtualsC1Ev
// CHECK-CTORS: call i8* @llvm.invariant.group.barrier(
// CHECK-CTORS-LABEL: }


// CHECK-NEW-LABEL: define void @_Z9Pointers1v()
// CHECK-NEW-NOT: @llvm.invariant.group.barrier(
// CHECK-NEW-LABEL: call void @_ZN12DynamicBase1C1Ev(

// CHECK-NEW: %[[THIS3:.*]] = call i8* @llvm.invariant.group.barrier(i8* %[[THIS2:.*]])
// CHECK-NEW: %[[THIS4:.*]] = bitcast i8* %[[THIS3]] to %[[DynamicDerived:.*]]*
// CHECK-NEW: call void @_ZN14DynamicDerivedC1Ev(%[[DynamicDerived:.*]]* %[[THIS4]])
// CHECK-NEW-LABEL: }
void Pointers1() {
  DynamicBase1 *DB = new DynamicBase1;
  DB->foo();

  DynamicDerived *DD = new (DB) DynamicDerived;
  DD->foo();
  DD->~DynamicDerived();
}

// CHECK-NEW-LABEL: define void @_Z14HackingObjectsv()
// CHECK-NEW:  call void @_ZN12DynamicBase1C1Ev
// CHECK-NEW:  call i8* @llvm.invariant.group.barrier(
// CHECK-NEW:  call void @_ZN14DynamicDerivedC1Ev(
// CHECK-NEW:  call i8* @llvm.invariant.group.barrier(
// CHECK-NEW: call void @_ZN12DynamicBase1C1Ev(
// CHECK-NEW-LABEL: }
void HackingObjects() {
  DynamicBase1 DB;
  DB.foo();

  DynamicDerived *DB2 = new (&DB) DynamicDerived;
  // Using DB now is prohibited.
  DB2->foo();
  DB2->~DynamicDerived();

  // We have to get back to the previous type to avoid calling wrong destructor
  new (&DB) DynamicBase1;
  DB.foo();
}

/*** Testing Constructors ***/
struct DynamicBase1;
// CHECK-CTORS-LABEL: define linkonce_odr void @_ZN12DynamicBase1C2Ev(
// CHECK-CTORS-NOT: call i8* @llvm.invariant.group.barrier(
// CHECK-CTORS-LABEL: }


struct DynamicDerived;

// CHECK-CTORS-LABEL: define linkonce_odr void @_ZN14DynamicDerivedC2Ev(
// CHECK-CTORS: %[[THIS0:.*]] = load %[[DynamicDerived:.*]]*, %[[DynamicDerived]]** {{.*}}
// CHECK-CTORS: %[[THIS1:.*]] = bitcast %[[DynamicDerived:.*]]* %[[THIS0]] to i8*
// CHECK-CTORS: %[[THIS2:.*]] = call i8* @llvm.invariant.group.barrier(i8* %[[THIS1:.*]])
// CHECK-CTORS: %[[THIS3:.*]] = bitcast i8* %[[THIS2]] to %[[DynamicDerived]]*
// CHECK-CTORS: %[[THIS4:.*]] = bitcast %[[DynamicDerived]]* %[[THIS3]] to %[[DynamicBase:.*]]*
// CHECK-CTORS: call void @_ZN12DynamicBase1C2Ev(%[[DynamicBase]]* %[[THIS4]])

// CHECK-CTORS: %[[THIS5:.*]] = bitcast %struct.DynamicDerived* %[[THIS0]] to i32 (...)***
// CHECK-CTORS: store {{.*}} %[[THIS5]]
// CHECK-CTORS-LABEL: }

struct DynamicDerivedMultiple;
// CHECK-CTORS-LABEL: define linkonce_odr void @_ZN22DynamicDerivedMultipleC2Ev(

// CHECK-CTORS: %[[THIS0:.*]] = load %[[CLASS:.*]]*, %[[CLASS]]** {{.*}}
// CHECK-CTORS: %[[THIS1:.*]] = bitcast %[[CLASS:.*]]* %[[THIS0]] to i8*
// CHECK-CTORS: %[[THIS2:.*]] = call i8* @llvm.invariant.group.barrier(i8* %[[THIS1]])
// CHECK-CTORS: %[[THIS3:.*]] = bitcast i8* %[[THIS2]] to %[[CLASS]]*
// CHECK-CTORS: %[[THIS4:.*]] = bitcast %[[CLASS]]* %[[THIS3]] to %[[BASE_CLASS:.*]]*
// CHECK-CTORS: call void @_ZN12DynamicBase1C2Ev(%[[BASE_CLASS]]* %[[THIS4]])

// CHECK-CTORS: call i8* @llvm.invariant.group.barrier(

// CHECK-CTORS: call void @_ZN12DynamicBase2C2Ev(
// CHECK-CTORS-NOT: @llvm.invariant.group.barrier


// CHECK-CTORS: %[[THIS10:.*]] = bitcast %struct.DynamicDerivedMultiple* %[[THIS0]] to i32 (...)***
// CHECK-CTORS: store {{.*}} @_ZTV22DynamicDerivedMultiple, i64 0, i64 2) {{.*}} %[[THIS10]]
// CHECK-CTORS: %[[THIS11:.*]] = bitcast %struct.DynamicDerivedMultiple* %[[THIS0]] to i8*
// CHECK-CTORS: %[[THIS_ADD:.*]] = getelementptr inbounds i8, i8* %[[THIS11]], i64 16
// CHECK-CTORS: %[[THIS12:.*]]  = bitcast i8* %[[THIS_ADD]] to i32 (...)***


// CHECK-CTORS: store {{.*}} @_ZTV22DynamicDerivedMultiple, i64 0, i64 6) {{.*}} %[[THIS12]]
// CHECK-CTORS-LABEL: }

struct DynamicFromStatic;
// CHECK-CTORS-LABEL: define linkonce_odr void @_ZN17DynamicFromStaticC2Ev(
// CHECK-CTORS-NOT: @llvm.invariant.group.barrier(
// CHECK-CTORS-LABEL: }


/** DTORS **/
// CHECK-DTORS-LABEL: define linkonce_odr void @_ZN10StaticBaseD2Ev(
// CHECK-DTORS-NOT: call i8* @llvm.invariant.group.barrier(
// CHECK-DTORS-LABEL: }


// CHECK-DTORS-LABEL: define linkonce_odr void @_ZN25DynamicFromVirtualStatic2D2Ev(
// CHECK-DTORS-NOT: invariant.barrier
// CHECK-DTORS-LABEL: }

// CHECK-DTORS-LABEL: define linkonce_odr void @_ZN17DynamicFromStaticD2Ev
// CHECK-DTORS-NOT: call i8* @llvm.invariant.group.barrier(
// CHECK-DTORS-LABEL: }


// CHECK-DTORS-LABEL: define linkonce_odr void @_ZN22DynamicDerivedMultipleD2Ev(

// CHECK-DTORS-LABEL: define linkonce_odr void @_ZN12DynamicBase2D2Ev(
// CHECK-DTORS: call i8* @llvm.invariant.group.barrier(
// CHECK-DTORS-LABEL: }

// CHECK-DTORS-LABEL: define linkonce_odr void @_ZN12DynamicBase1D2Ev
// CHECK-DTORS: call i8* @llvm.invariant.group.barrier(
// CHECK-DTORS-LABEL: }

// CHECK-DTORS-LABEL: define linkonce_odr void @_ZN14DynamicDerivedD2Ev
// CHECK-DTORS-NOT: call i8* @llvm.invariant.group.barrier(
// CHECK-DTORS-LABEL: }


// CHECK-LINK-REQ: !llvm.module.flags = !{![[FIRST:.*]], ![[SEC:.*]]{{.*}}}

// CHECK-LINK-REQ: ![[FIRST]] = !{i32 1, !"StrictVTablePointers", i32 1}
// CHECK-LINK-REQ: ![[SEC]] = !{i32 3, !"StrictVTablePointersRequirement", ![[META:.*]]}
// CHECK-LINK-REQ: ![[META]] = !{!"StrictVTablePointers", i32 1}

