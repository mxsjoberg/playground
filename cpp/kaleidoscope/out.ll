; ModuleID = 'llvm codegen'
source_filename = "llvm codegen"

@0 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

define i32 @main() {
entry:
  %calltmp = call double @cos(double 1.234000e+00)
  %printfCall = call i32 (i8*, double, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), double %calltmp)
  ret i32 0
}

declare double @cos(double)

declare i32 @printf(i8*, double, ...)
