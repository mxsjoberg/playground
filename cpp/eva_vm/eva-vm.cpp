#include "src/EvaVM.h"

/**
 * Eva VM main executable
 */

int main(int argc, char const *argv[]) {
	EvaVM vm;

	vm.exec(R"(
		42
	)");
}

/*
	$ clang++ -v
		Apple clang version 14.0.3 (clang-1403.0.22.14.1)
		Target: x86_64-apple-darwin22.4.0
	
	$ clang++ -std=c++17 -Wall -ggdb3 ./eva-vm.cpp -o ./eva-vm
*/