#import <string>

#import "./src/EvaLLVM.h"

int main(int argc, char const *argv[]) {
    // example program
    std::string program = R"(42)";
    
    // compiler instance
    EvaLLVM vm;
    
    // generate LLVM IR
    vm.exec(program);
}