#import <string>

#import "./src/EvaLLVM.h"

int main(int argc, char const *argv[]) {

	std::string program = R"(
		42
	)";

	EvaLLVM vm;

	vm.exec(program);
	// ; ModuleID = 'EvaLLVM'
	// source_filename = "EvaLLVM"

	return 0;
}