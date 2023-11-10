#ifndef EvaLLVM_h
#define EvaLLVM_h

#import <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class EvaLLVM {
public:
	EvaLLVM() { moduleInit(); }

	void exec(const std::string& program) {
		// parse to AST
		// compile to LLVM IR

		// print generated code
		module->print(llvm::outs(), nullptr);

		// save to file out.ll
		saveModuleToFile("out.ll");
	}

private:
	// save module to file
	void saveModuleToFile(const std::string& filename) {
		std::error_code error;
		llvm::raw_fd_ostream outLL(filename, error);
		module->print(outLL, nullptr);
	}

	// init module
	void moduleInit() {
		context = std::make_unique<llvm::LLVMContext>();
		module = std::make_unique<llvm::Module>("EvaLLVM", *context);
		builder = std::make_unique<llvm::IRBuilder<>>(*context);
	}
	// LLVM context
	std::unique_ptr<llvm::LLVMContext> context;
	// LLVM module
	std::unique_ptr<llvm::Module> module;
	// LLVM builder
	std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif