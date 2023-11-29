#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"

#define log(message) std::cout << message << std::endl;
#define log_error(message, lexeme) std::cout << "error : " << message << " : " << lexeme << std::endl; exit(1);

// --------------------------------------------------------
// lexer

enum class TokenType {
    tok_bof,
    tok_eof,
    tok_number,
    tok_plus,
    tok_minus,
    tok_multiply,
    tok_divide,
    tok_semicolon,
    tok_def,
    tok_extern,
    tok_identifier,
    tok_lparen,
    tok_rparen,
};

struct Token {
    TokenType type;
    std::string lexeme;
    // values
    double number;
    std::string identifier;
};

std::string token_to_string(const Token& token, bool include_token = true) {
    switch (token.type) {
        case TokenType::tok_bof:
            return "< BOF >";
        case TokenType::tok_eof:
            return "";
        case TokenType::tok_number:
            return "tok_number(" + token.lexeme + ")";
        case TokenType::tok_plus:
            return "tok_plus(+)";
        case TokenType::tok_minus:
            return "tok_minus(-)";
        case TokenType::tok_multiply:
            return "tok_multiply(*)";
        case TokenType::tok_divide:
            return "tok_divide(/)";
        case TokenType::tok_semicolon:
            return "tok_semicolon(;)";
        case TokenType::tok_def:
            return "tok_def(" + token.identifier + ")";
        case TokenType::tok_extern:
            return "tok_extern(" + token.identifier + ")";
        case TokenType::tok_identifier:
            return include_token ? "tok_identifier(" + token.identifier + ")" : token.identifier;
        case TokenType::tok_lparen:
            return "tok_lparen";
        case TokenType::tok_rparen:
            return "tok_rparen";
        default:
            return "?";
    }
}

std::vector<Token> tokenize(const std::string& source) {
    std::vector<Token> tokens;
    size_t ch_index = 0;
    while (ch_index < source.size()) {
        char ch = source[ch_index];
        switch (ch) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                ch_index++;
                break;
            case ';':
                tokens.push_back({ TokenType::tok_semicolon, ";" });
                ch_index++;
                break;
            case '+':
                tokens.push_back({ TokenType::tok_plus, "+" });
                ch_index++;
                break;
            case '-':
                tokens.push_back({ TokenType::tok_minus, "-" });
                ch_index++;
                break;
            case '*':
                tokens.push_back({ TokenType::tok_multiply, "*" });
                ch_index++;
                break;
            case '/':
                tokens.push_back({ TokenType::tok_divide, "/" });
                ch_index++;
                break;
            case '(':
                tokens.push_back({ TokenType::tok_lparen, "(" });
                ch_index++;
                break;
            case ')':
                tokens.push_back({ TokenType::tok_rparen, ")" });
                ch_index++;
                break;
            default:
                if (isdigit(ch)) {
                    size_t start = ch_index;
                    while ((isdigit(source[ch_index]) || source[ch_index] == '.' ) && ch_index < source.size()) {
                        ch_index++;
                    }
                    std::string lexeme = source.substr(start, ch_index - start);
                    tokens.push_back({ TokenType::tok_number, lexeme, std::stod(lexeme) });
                } else if (isalpha(ch)) {
                    size_t start = ch_index;
                    while (isalnum(source[ch_index]) && ch_index < source.size()) {
                        ch_index++;
                    }
                    std::string lexeme = source.substr(start, ch_index - start);
                    if (lexeme == "def") {
                        tokens.push_back({ TokenType::tok_def, lexeme });
                    } else if (lexeme == "extern") {
                        tokens.push_back({ TokenType::tok_extern, lexeme });
                    } else {
                        // identifier
                        tokens.push_back({ TokenType::tok_identifier, lexeme, 0, lexeme });
                    }
                } else {
                    log_error("tokenize : unexpected character", ch);
                    ch_index++;
                }
        }
    }
    return tokens;
}

// --------------------------------------------------------
// ast

struct AST {
    Token token;
    std::vector<AST> children;
    std::vector<Token> arguments;
};

void print_tree(const AST& node, int level = 0) {
    std::cout << std::string(level * 2, ' ') << token_to_string(node.token);
    if (node.arguments.size() > 0) {
        std::string arguments = "";
        for (auto& argument : node.arguments) {
            bool is_last = token_to_string(node.arguments.back()) == token_to_string(argument);
            arguments.append(token_to_string(argument, false));
            if (!is_last) {
                arguments.append(",");
            }
        }
        std::cout << " [" << arguments << "]" << std::endl;
    } else {
        std::cout << std::endl;
    }
    for (const AST& child : node.children) {
        print_tree(child, level + 1);
    }
}

// --------------------------------------------------------
// parser

AST parse_expression(const std::vector<Token>& tokens, size_t& token_index);

// parse : factor ::= tok_number | tok_identifier
AST parse_factor(const std::vector<Token>& tokens, size_t& token_index) {
    AST expression;
    AST factor;
    Token token = tokens[token_index];
    switch (token.type) {
        case TokenType::tok_number:
            token_index++;
            return AST { token };
        case TokenType::tok_identifier:
            token_index++;
            // function call
            if (tokens[token_index].type == TokenType::tok_lparen) {
                token_index++;
                std::vector<AST> arguments;
                while (tokens[token_index].type != TokenType::tok_rparen) {
                    arguments.push_back(parse_factor(tokens, token_index));
                }
                token_index++;
                return AST { token, arguments };
            } else {
                return AST { token };
            }
            return AST { token };
        default:
            log_error("parse_factor : unexpected token", token.lexeme);
            token_index++;
            return AST { token };
    }
}
// parse : term ::= factor (('*' | '/') factor)*
AST parse_term(const std::vector<Token>& tokens, size_t& token_index) {
    AST term = parse_factor(tokens, token_index);
    while (tokens[token_index].type == TokenType::tok_multiply || tokens[token_index].type == TokenType::tok_divide) {
        Token token = tokens[token_index];
        token_index++;
        // factor
        auto children = { term, parse_factor(tokens, token_index) };
        term = AST { token, children };
    }
    return term;
}
// parse : expression ::= term ('+' term)* ';'
AST parse_expression(const std::vector<Token>& tokens, size_t& token_index) {
    AST expression = parse_term(tokens, token_index);
    while (tokens[token_index].type == TokenType::tok_plus || tokens[token_index].type == TokenType::tok_minus) {
        Token token = tokens[token_index];
        token_index++;
        // term
        auto children = { expression, parse_term(tokens, token_index) };
        expression = AST { token, children };
    }
    // semicolon
    if (tokens[token_index].type != TokenType::tok_semicolon) {
        log_error("parse_expression : expected ';'", tokens[token_index].lexeme);
    }
    token_index++;
    return expression;
}
// parse : definition :== 'def' tok_identifier '(' (tok_identifier)+ ')' expression
AST parse_definition(const std::vector<Token>& tokens, size_t& token_index) {
    token_index++;
    // function name
    Token fname = { TokenType::tok_def, "", 0, tokens[token_index].identifier };
    token_index++;
    // arguments
    if (tokens[token_index].type != TokenType::tok_lparen) {
        log_error("parse_definition : expected '('", tokens[token_index].lexeme);
    }
    token_index++;
    std::vector<Token> arguments;
    while (tokens[token_index].type == TokenType::tok_identifier) {
        Token token = tokens[token_index];
        token_index++;
        arguments.push_back(token);
    }
    if (tokens[token_index].type != TokenType::tok_rparen) {
        log_error("parse_definition : expected ')'", tokens[token_index].lexeme);
    }
    token_index++;
    // function body
    AST expression = parse_expression(tokens, token_index);
    return AST { fname, { expression }, arguments };
}

// parse : external :== 'extern' tok_identifier '(' (tok_identifier)+ ')' ';'
AST parse_external(const std::vector<Token>& tokens, size_t& token_index) {
    token_index++;
    // function name
    Token fname = { TokenType::tok_extern, "", 0, tokens[token_index].identifier };
    token_index++;
    // arguments
    if (tokens[token_index].type != TokenType::tok_lparen) {
        log_error("parse_external : expected '('", tokens[token_index].lexeme);
    }
    token_index++;
    std::vector<Token> arguments;
    while (tokens[token_index].type == TokenType::tok_identifier) {
        Token token = tokens[token_index];
        token_index++;
        arguments.push_back(token);
    }
    if (tokens[token_index].type != TokenType::tok_rparen) {
        log_error("parse_external : expected ')'", tokens[token_index].lexeme);
    }
    token_index++;
    // semicolon
    if (tokens[token_index].type != TokenType::tok_semicolon) {
        log_error("parse_external : expected ';'", tokens[token_index].lexeme);
    }
    token_index++;
    return AST { fname, {}, arguments };
}

// parse
AST parse(const std::vector<Token>& tokens) {
    size_t token_index = 0;
    AST program;
    program.token = Token { TokenType::tok_bof, "" };
    while (token_index < tokens.size()) {
        if (tokens[token_index].type == TokenType::tok_def) {
            program.children.push_back(parse_definition(tokens, token_index));
        } else if (tokens[token_index].type == TokenType::tok_extern) {
            program.children.push_back(parse_external(tokens, token_index));
        } else {
            program.children.push_back(parse_expression(tokens, token_index));
        }
    }
    return program;
}

// --------------------------------------------------------
// codegen

static std::unique_ptr<llvm::LLVMContext> context;
static std::unique_ptr<llvm::Module> module;
static std::unique_ptr<llvm::IRBuilder<>> builder;

static std::map<std::string, llvm::Value *> named_values;

llvm::Value *codegen(const AST& tree);

llvm::Value *log_value(const std::string message) {
    log(message);
    return nullptr;
}

llvm::Value *log_error_value(const std::string message, const std::string lexeme) {
    log_error(message, lexeme);
    return nullptr;
}

// codegen : binop
llvm::Value *codegen_binop(const AST& tree) {
    llvm::Value *left = codegen(tree.children[0]);
    llvm::Value *right = codegen(tree.children[1]);
    if (!left || !right) {
        return nullptr;
    }
    if (tree.token.type == TokenType::tok_plus) {
        return builder->CreateFAdd(left, right, "addtmp");
    }
    if (tree.token.type == TokenType::tok_minus) {
        return builder->CreateFSub(left, right, "subtmp");
    }
    if (tree.token.type == TokenType::tok_multiply) {
        return builder->CreateFMul(left, right, "multmp");
    }
    if (tree.token.type == TokenType::tok_divide) {
        return builder->CreateFDiv(left, right, "divtmp");
    }
    return log_error_value("codegen_binop : unexpected token", token_to_string(tree.token));
}
// codegen : tok_number
llvm::Value *codegen_number(const AST& tree) {
    // return llvm::ConstantInt::get(*context, llvm::APInt(32, tree.token.number, true));
    return llvm::ConstantFP::get(*context, llvm::APFloat(tree.token.number));
}

// codegen : tok_identifier
llvm::Value *codegen_identifier(const AST& tree) {
    if (tree.children.size() > 0) {
        // function call
        llvm::Function *function = module->getFunction(tree.token.identifier);
        // arguments
        std::vector<llvm::Value *> arguments;
        for (const AST& child : tree.children) {
            arguments.push_back(codegen(child));
        }
        return builder->CreateCall(function, arguments, "calltmp");
    } else {
        auto named_value = named_values.find(tree.token.identifier);
        if (named_value != named_values.end()) {
            return named_value->second;
        } else {
            return log_error_value("codegen_identifier : unknown identifier", tree.token.identifier);
        }
    }
}

// codegen : tok_def
llvm::Function *codegen_function(const AST& tree) {
    llvm::BasicBlock *main_entry_block = builder->GetInsertBlock();
    // define new function
    std::vector<llvm::Type *> argument_types(tree.arguments.size(), builder->getDoubleTy());
    llvm::FunctionType *function_type = llvm::FunctionType::get(builder->getDoubleTy(), argument_types, false);
    llvm::Function *function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, tree.token.identifier, module.get());
    // set parameters
    llvm::Function::arg_iterator function_arguments = function->arg_begin();
    for (auto &argument : tree.arguments) {
        llvm::Value *llvm_argument = &*function_arguments++;
        llvm_argument->setName(argument.identifier);
        named_values[argument.identifier] = llvm_argument;
    }
    // create entry block
    llvm::BasicBlock *entry_block = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(entry_block);
    // function body
    llvm::Value *body = codegen(tree.children[0]);
    builder->CreateRet(body);
    // move back to main entry block
    builder->SetInsertPoint(main_entry_block);
    return function;
}

// codegen : tok_extern
llvm::Function *codegen_extern(const AST& tree) {
    llvm::BasicBlock *main_entry_block = builder->GetInsertBlock();
    // define new function
    std::vector<llvm::Type *> argument_types(tree.arguments.size(), builder->getDoubleTy());
    llvm::FunctionType *function_type = llvm::FunctionType::get(builder->getDoubleTy(), argument_types, false);
    llvm::Function *function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, tree.token.identifier, module.get());
    // set parameters
    llvm::Function::arg_iterator function_arguments = function->arg_begin();
    for (auto &argument : tree.arguments) {
        llvm::Value *llvm_argument = &*function_arguments++;
        llvm_argument->setName(argument.identifier);
        named_values[argument.identifier] = llvm_argument;
    }
    // move back to main entry block
    builder->SetInsertPoint(main_entry_block);
    return function;
}

// codegen
llvm::Value *codegen(const AST& tree) {
    llvm::Value *result = nullptr;
    switch (tree.token.type) {
    case TokenType::tok_bof:
        for (const AST& child : tree.children) {
            result = codegen(child);
        }
        return result;
    case TokenType::tok_eof:
        return nullptr;
    case TokenType::tok_number:
        return codegen_number(tree);
    case TokenType::tok_plus:
    case TokenType::tok_minus:
    case TokenType::tok_multiply:
    case TokenType::tok_divide:
        return codegen_binop(tree);
    case TokenType::tok_def:
        codegen_function(tree);
        return nullptr;
    case TokenType::tok_extern:
        codegen_extern(tree);
        return nullptr;
    case TokenType::tok_identifier:
        return codegen_identifier(tree);
    default:
        return log_error_value("codegen : unexpected token", token_to_string(tree.token));
    }
}

// --------------------------------------------------------
// driver

// clang++ -g -O0 fparser.cpp -o fparser `llvm-config --cxxflags --ldflags --system-libs --libs core` && ./fparser
// clang++ -g -O0 fparser.cpp -o fparser `llvm-config --cxxflags --ldflags --system-libs --libs core` && ./fparser > out.ll 
// lli out.ll; echo $?
int main() {
    // std::string program = R"(
    //     def foo(a b) a*a + 2*a*b + b*b;
    //     foo(1 2);
    // )";
    std::string program = R"(
        extern cos(x);
        cos(1.234);
    )";
    std::regex pattern(R"(^\s+)", std::regex::multiline);
    std::string source = std::regex_replace(program, pattern, "");
    // tokens
    std::vector<Token> tokens = tokenize(source);
    // for (auto &token : tokens) { std::cout << token_to_string(token) << std::endl; }
    // ast
    AST tree = parse(tokens);
    // print_tree(tree);
    // llvm
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("llvm codegen", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    // create main function
    llvm::FunctionType *function_type = llvm::FunctionType::get(builder->getInt32Ty(), false);
    llvm::Function *function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, "main", module.get());
    llvm::BasicBlock *main_entry_block = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(main_entry_block);
    // generate llvm ir
    llvm::Value *result = codegen(tree);
    if (result != nullptr) {
        // declare printf function
        std::vector<llvm::Type*> printf_arguments;
        printf_arguments.push_back(builder->getInt8PtrTy());
        printf_arguments.push_back(llvm::Type::getDoubleTy(*context));
        llvm::FunctionType *printf_function_type = llvm::FunctionType::get(builder->getInt32Ty(), printf_arguments, true);
        llvm::Function *printf_function = llvm::Function::Create(printf_function_type, llvm::Function::ExternalLinkage, "printf", module.get());
        // create the printf call
        std::vector<llvm::Value*> printf_values;
        printf_values.push_back(builder->CreateGlobalStringPtr("%f\n"));
        printf_values.push_back(result);
        builder->CreateCall(printf_function, printf_values, "printfCall");
    }
    // set entry block
    builder->SetInsertPoint(main_entry_block);
    // create return instruction
    builder->CreateRet(builder->getInt32(0));
    // verify
    llvm::verifyFunction(*function);
    // print llvm ir
    module->print(llvm::outs(), nullptr);
}
