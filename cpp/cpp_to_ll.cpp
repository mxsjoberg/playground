// clang -S -emit-llvm cpp_to_ll.cpp
int main() {
	int x = 15;
	return x + 10 - 5;
}

// cpp_to_ll.ll
//
// define i32 @main() #0 {
//   %1 = alloca i32, align 4
//   %2 = alloca i32, align 4
//   store i32 0, i32* %1, align 4
//   store i32 15, i32* %2, align 4
//   %3 = load i32, i32* %2, align 4
//   %4 = add nsw i32 %3, 10
//   %5 = sub nsw i32 %4, 5
//   ret i32 %5
// }
