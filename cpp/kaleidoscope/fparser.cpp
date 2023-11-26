#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/APFloat.h"

#define log_error(message, lexeme) std::cout << message << " : " << lexeme << std::endl;

// --------------------------------------------------------
// lexer

enum class TokenType {
    tok_number,
    tok_plus,
    tok_minus,
    tok_multiply,
    tok_divide,
    tok_semicolon,
    tok_bof,
    tok_eof,
};

struct Token {
    TokenType type;
    std::string lexeme;
    double number;
};

std::string token_to_string(const Token& token) {
    switch (token.type) {
        case TokenType::tok_bof:
            return "< BOF >";
        case TokenType::tok_eof:
            return "";
        case TokenType::tok_semicolon:
            return "tok_semicolon(;)";
        case TokenType::tok_plus:
            return "tok_plus(+)";
        case TokenType::tok_minus:
            return "tok_minus(-)";
        case TokenType::tok_multiply:
            return "tok_multiply(*)";
        case TokenType::tok_divide:
            return "tok_divide(/)";
        case TokenType::tok_number:
            return "tok_number(" + token.lexeme + ")";
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
            default:
                if (isdigit(ch)) {
                    size_t start = ch_index;
                    while (isdigit(source[ch_index]) && ch_index < source.size()) {
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
                    log_error("tokenize : unexpected identifier", lexeme);
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
};

void print_tree(const AST& node, int level = 0) {
    std::cout << std::string(level * 2, ' ') << token_to_string(node.token) << std::endl;
    for (const AST& child : node.children) {
        print_tree(child, level + 1);
    }
}

// --------------------------------------------------------
// parser

// parse : factor ::= '+' factor | '-' factor | tok_number | '(' expression ')'
AST parse_factor(const std::vector<Token>& tokens, size_t& token_index) {
    AST expression;
    AST factor;
    Token token = tokens[token_index];
    switch (token.type) {
        case TokenType::tok_semicolon:
            token_index++;
            return expression;
        case TokenType::tok_plus:
        case TokenType::tok_minus:
            factor.token = token;
            token_index++;
            factor.children.push_back(parse_factor(tokens, token_index));
            return factor;
        case TokenType::tok_number:
            token_index++;
            return AST{ token };
        // LPAR expression RPAR
        // case TokenType::LPAR:
        //     // LPAR
        //     token_index++;
        //     // expression
        //     expression = parse_expression(tokens, token_index);
        //     // RPAR
        //     if (tokens[token_index].type != TokenType::RPAR) {
        //         throw std::runtime_error("Expected ')'");
        //     }
        //     token_index++;
        //     return expression;
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
        term = AST{ token, children };
    }
    return term;
}

// parse : expression ::= term ('+' term)* ;
AST parse_expression(const std::vector<Token>& tokens, size_t& token_index) {
    AST expression = parse_term(tokens, token_index);
    while (tokens[token_index].type == TokenType::tok_plus || tokens[token_index].type == TokenType::tok_minus) {
        Token token = tokens[token_index];
        token_index++;
        // term
        auto children = { expression, parse_term(tokens, token_index) };
        expression = AST{ token, children };
        // semicolon
        if (tokens[token_index].type != TokenType::tok_semicolon) {
            log_error("parse_expression : expected ';'", tokens[token_index].lexeme);
            break;
        }
        token_index++;
    }
    return expression;
}

// parse
AST parse(const std::vector<Token>& tokens) {
    size_t token_index = 0;
    AST program;
    program.token = Token{ TokenType::tok_bof, "" };
    while (token_index < tokens.size()) {
        program.children.push_back(parse_expression(tokens, token_index));
    }
    return program;
}

// --------------------------------------------------------
// codegen

static std::unique_ptr<llvm::LLVMContext> context;
static std::unique_ptr<llvm::Module> module;
static std::unique_ptr<llvm::IRBuilder<>> builder;

llvm::Value *codegen(const AST& tree);

llvm::Value *log_error_value(const std::string message, const std::string lexeme) {
    log_error(message, lexeme);
    return nullptr;
}

// codegen : tok_plus
llvm::Value *codegen_plus(const AST& tree) {
    llvm::Value *left = codegen(tree.children[0]);
    llvm::Value *right = codegen(tree.children[1]);
    if (!left || !right) {
        return nullptr;
    }
    return builder->CreateFAdd(left, right, "addtmp");
}

// codegen : tok_minus
llvm::Value *codegen_minus(const AST& tree) {
    llvm::Value *left = codegen(tree.children[0]);
    llvm::Value *right = codegen(tree.children[1]);
    if (!left || !right) {
        return nullptr;
    }
    return builder->CreateFSub(left, right, "subtmp");
}

// codegen : tok_multiply
llvm::Value *codegen_multiply(const AST& tree) {
    llvm::Value *left = codegen(tree.children[0]);
    llvm::Value *right = codegen(tree.children[1]);
    if (!left || !right) {
        return nullptr;
    }
    return builder->CreateFMul(left, right, "multmp");
}

// codegen : tok_divide
llvm::Value *codegen_divide(const AST& tree) {
    llvm::Value *left = codegen(tree.children[0]);
    llvm::Value *right = codegen(tree.children[1]);
    if (!left || !right) {
        return nullptr;
    }
    return builder->CreateFDiv(left, right, "divtmp");
}

// codegen : tok_number
llvm::Value *codegen_number(const AST& tree) {
    return llvm::ConstantFP::get(*context, llvm::APFloat(tree.token.number));
}

// codegen
llvm::Value *codegen(const AST& tree) {
    switch (tree.token.type) {
    case TokenType::tok_bof:
        for (const AST& child : tree.children) {
            return codegen(child);
        }
        return nullptr;
    case TokenType::tok_eof:
        return nullptr;
    case TokenType::tok_number:
        // std::cout << "codegen : tok_number" << std::endl;
        return codegen_number(tree);
    case TokenType::tok_plus:
        // std::cout << "codegen : tok_plus" << std::endl;
        return codegen_plus(tree);
    case TokenType::tok_minus:
        // std::cout << "codegen : tok_minus" << std::endl;
        return codegen_minus(tree);
    case TokenType::tok_multiply:
        // std::cout << "codegen : tok_multiply" << std::endl;
        return codegen_multiply(tree);
    case TokenType::tok_divide:
        // std::cout << "codegen : tok_divide" << std::endl;
        return codegen_divide(tree);
    default:
        return log_error_value("codegen : unexpected token", token_to_string(tree.token));
    }
}

// --------------------------------------------------------
// driver

// clang++ -g -O0 fparser.cpp -o fparser `llvm-config --cxxflags --ldflags --system-libs --libs core`; ./fparser
int main() {
    std::string program = R"(
        4+5*2;
    )";
    /*
    ; ModuleID = 'llvm codegen'
    source_filename = "llvm codegen"

    define double @__anon_expr() {
    entry:
      ret double 1.400000e+01
    }
    */
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
    // generate llvm ir
    llvm::Value *result = codegen(tree);
    // create anonymous function as entry point
    llvm::FunctionType *function_type = llvm::FunctionType::get(builder->getDoubleTy(), false);
    llvm::Function *function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, "__anon_expr", module.get());
    llvm::BasicBlock *entry_block = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(entry_block);
    builder->CreateRet(result);

    module->print(llvm::errs(), nullptr);
}
