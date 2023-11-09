#ifndef EvaVM_h
#define EvaVM_h

#include <string>
#include <vector>
#include <array>

#include "./Logger.h"
#include "./OpCode.h"
#include "./EvaValue.h"

// stack size limit
#define STACK_LIMIT 512

#define READ_BYTE() *ip++

class EvaVM {
public:
    EvaVM() {}

    /**
     * Push a value to stack
     * @param value The value to push
     */
     void push(const EvaValue& value) {
        if ((size_t)(sp - stack.begin()) == STACK_LIMIT) {
            DIE << "stack overflow\n";
        }
        *sp = value;
        sp++;
     }

     /**
      * Pop a value from stack
      * @return The value popped
      */
     EvaValue pop() {
        if (sp == stack.begin()) {
            DIE << "empty stack\n";
        }
        sp--;
        return *sp;
     }

    /**
     * Execute a program
     * @param program The program to execute
     */
    EvaValue exec(const std::string &program) {
        // auto ast = parser->parse(program)
        // code = compiler->Compile(ast)
        
        // for testing only
        // constants.push_back(NUMBER(10));
        // constants.push_back(NUMBER(3));
        // constants.push_back(NUMBER(10));
        // code = { OP_CONST, 0, OP_CONST, 1, OP_MUL, OP_CONST, 2, OP_SUB, OP_HALT };

        constants.push_back(ALLOC_STRING("hello "));
        constants.push_back(ALLOC_STRING("eva"));
        code = { OP_CONST, 0, OP_CONST, 1, OP_ADD, OP_HALT };

        // init pointers
        ip = &code[0];
        sp = &stack[0];

        return eval();
    }

    /*
     * Evaluate a program
     * @return The result of evaluation
     */
    EvaValue eval() {
        for (;;) {
            auto opcode = READ_BYTE();
            // log(opcode);
            switch (opcode) {
                case OP_HALT:
                    return pop();
                case OP_CONST:
                    push(constants[READ_BYTE()]);
                    break;
                case OP_ADD: {
                    // LIFO
                    // auto right = pop().number;
                    // auto left = pop().number;
                    // auto result = left + right;
                    // push(NUMBER(result));
                    auto right = pop();
                    auto left = pop();
                    // numbers
                    if (IS_NUMBER(right) && IS_NUMBER(left)) {
                        push(NUMBER(AS_NUMBER(left) + AS_NUMBER(right)));
                    }
                    // strings
                    else if (IS_STRING(right) && IS_STRING(left)) {
                        push(ALLOC_STRING(AS_STRING(left) + AS_STRING(right)));
                    }
                    break;
                }
                case OP_SUB: {
                    // LIFO
                    auto right = pop().number;
                    auto left = pop().number;
                    auto result = left - right;
                    push(NUMBER(result));
                    break;
                }
                case OP_MUL: {
                    // LIFO
                    auto right = pop().number;
                    auto left = pop().number;
                    auto result = left * right;
                    push(NUMBER(result));
                    break;
                }
                case OP_DIV: {
                    // LIFO
                    auto right = pop().number;
                    auto left = pop().number;
                    auto result = left / right;
                    push(NUMBER(result));
                    break;
                }
                default:
                    DIE << "unknown opcode: " << std::hex << opcode;
            }
        }
    }

    // pointers
    uint8_t* ip;
    EvaValue* sp;
    // stack
    std::array<EvaValue, STACK_LIMIT> stack;
    // constant pool
    std::vector<EvaValue> constants;
    // bytecode
    std::vector<uint8_t> code;
};

#endif