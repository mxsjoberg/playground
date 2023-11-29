; ModuleID = 'llvm codegen'
source_filename = "llvm codegen"

define i32 @main() {
entry:
  %calltmp = call i32 @foo(i32 1, i32 2)
  ret i32 %calltmp
}

define i32 @foo(i32 %a, i32 %b) {
entry:
  %multmp = mul i32 %a, %a
  %multmp1 = mul i32 2, %a
  %multmp2 = mul i32 %multmp1, %b
  %addtmp = add i32 %multmp, %multmp2
  %multmp3 = mul i32 %b, %b
  %addtmp4 = add i32 %addtmp, %multmp3
  ret i32 %addtmp4
}
