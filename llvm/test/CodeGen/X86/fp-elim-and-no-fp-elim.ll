; RUN: llc < %s -mtriple x86_64-apple-darwin | FileCheck %s

define void @bar(i32 %argc) #0 {
; CHECK: bar:
; CHECK: pushq %rbp
entry:
  %conv = sitofp i32 %argc to double
  %mul = fmul double %conv, 3.792700e+01
  %conv1 = fptrunc double %mul to float
  %div = fdiv double 9.273700e+02, %conv
  %conv3 = fptrunc double %div to float
  tail call void @foo(float %conv1, float %conv3)
  ret void
}

define void @qux(i32 %argc) #1 {
; CHECK: qux:
; CHECK-NOT: pushq %rbp
entry:
  %conv = sitofp i32 %argc to double
  %mul = fmul double %conv, 3.792700e+01
  %conv1 = fptrunc double %mul to float
  %div = fdiv double 9.273700e+02, %conv
  %conv3 = fptrunc double %div to float
  tail call void @foo(float %conv1, float %conv3)
  ret void
}

declare void @foo(float, float)

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
