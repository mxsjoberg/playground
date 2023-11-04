#ifndef __EvaVM_h
#define __EvaVM_h

#include <string>
#include <vector>

#include "./OpCode.h"

/**
 * Read current byte in bytecode and increment ip
 */
#define READ_BYTE() *ip++

/**
 * Virtual machine for the Eva language
 */
class EvaVM {
public:
    EvaVM() {}

    /**
     * Execute a program
     * @param program The program to execute
     */
    void exec(const std::string &program) {
        // parse program
        // auto ast = parser->parse(program)

        // compile to bytecode
        // code = compiler->Compile(ast)

        code = { OP_HALT };

        // set instruction pointer to 0
        ip = &code[0];

        return eval();
    }

    /**
     * Main eval loop
     */
    void eval() {
        for (;;) {
            switch (READ_BYTE()) {
                case OP_HALT: {
                    return;
                }
            }
        }
    }

    /**
     * Instruction pointer
     */
    uint8_t* ip;

    /**
     * Bytecode
     */
    std::vector<uint8_t> code;
};

#endif