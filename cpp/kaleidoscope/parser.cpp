// https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl04.html

/*
    ready> 4+5;
    handle_toplevel_expression
    define double @0() {
    entry:
      ret double 9.000000e+00
    }
*/

/*
    ready> def foo(a b) a*a + 2*a*b + b*b;
    handle_definition
    define double @foo(double %a, double %b) {
    entry:
      %multmp = fmul double %b, %b
      %multmp1 = fmul double 2.000000e+00, %b
      %multmp2 = fmul double %multmp1, %b
      %addtmp = fadd double %multmp, %multmp2
      %multmp3 = fmul double %b, %b
      %addtmp4 = fadd double %addtmp, %multmp3
      ret double %addtmp4
    }
*/

/*
    ready> def bar(a) foo(a, 4.0) + bar(31337);
    handle_definition
    define double @bar(double %a) {
    entry:
      %calltmp = call double @foo()
      %calltmp1 = call double @bar()
      %addtmp = fadd double %calltmp, %calltmp1
      ret double %addtmp
    }
*/

/*
    ready> extern cos(x);
    handle_extern
    declare double @cos(double)

    ready> cos(1.234);
    handle_toplevel_expression
    define double @__anon_expr() {
    entry:
      %calltmp = call double @cos()
      ret double %calltmp
    }
*/

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
// llvm
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

// lexer
// ---------------------------------------
enum Token {
    tok_eof = -1,
    // commands
    tok_def = -2,
    tok_extern = -3,
    // primary
    tok_identifier = -4,
    tok_number = -5,
};

static std::string identifier; // tok_identifier
static double number; // tok_number

static int next_token() {
    static int current_char = ' ';
    // skip whitespace
    while (isspace(current_char)) {
        current_char = getchar();
    }
    // identifier: [a-zA-Z]*
    if (isalpha(current_char)) {
        identifier = current_char;
        while (isalnum(current_char = getchar())) {
            identifier += current_char;
        }
        // keywords
        if (identifier == "def") {
            return tok_def;
        }
        if (identifier == "extern") {
            return tok_extern;
        }
        return tok_identifier;
    }
    // number: [0-9.]+
    if (isdigit(current_char) || current_char == '.') {
        std::string tmp_number;
        do {
            tmp_number += current_char;
            current_char = getchar();
        } while (isdigit(current_char) || current_char == '.');

        number = strtod(tmp_number.c_str(), nullptr);
        return tok_number;
    }
    // comments
    if (current_char == '#') {
        do {
            current_char = getchar();
        } while (current_char != EOF && current_char != '\n' && current_char != '\r');

        if (current_char != EOF) {
            return next_token();
        }
    }
    // end of file
    if (current_char == EOF) {
        return tok_eof;
    }
    // otherwise
    int unknown_char = current_char;
    current_char = getchar();
    return unknown_char;
}

// ast
// ---------------------------------------
namespace {

class ExprAST {
public:
    virtual ~ExprAST() = default; // virtual destructor
    // llvm
    virtual llvm::Value *codegen() = 0; // SSA value
};

class NumberExprAST: public ExprAST {
    double num;
public:
    NumberExprAST(double num) : num(num) {}
    // llvm
    llvm::Value *codegen() override;
};

class VariableExprAST : public ExprAST {
    std::string id;
public:
    VariableExprAST(const std::string &id) : id(id) {}
    // llvm
    llvm::Value *codegen() override;
};

class BinaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> lhs, rhs;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    // llvm
    llvm::Value *codegen() override;
};

class CallExprAST: public ExprAST {
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee(callee), args(std::move(args)) {}
    // llvm
    llvm::Value *codegen() override;
};

// functions
class PrototypeAST {
    std::string name;
    std::vector<std::string> args;
public:
    PrototypeAST(const std::string &name, std::vector<std::string> args)
        : name(name), args(std::move(args)) {}
    // llvm
    llvm::Function *codegen();
    // getter
    const std::string &get_identifier() const { return name; }
};

class FunctionAST {
    std::unique_ptr<PrototypeAST> prototype;
    std::unique_ptr<ExprAST> body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> prototype, std::unique_ptr<ExprAST> body)
        : prototype(std::move(prototype)), body(std::move(body)) {}
    // llvm
    llvm::Function *codegen();
};

} // end anonymous namespace

// parser
// ---------------------------------------
static int current_token;
static int get_next_token() { return current_token = next_token(); }

// binary operator precedence
static std::map<char, int> binop_precedence;
static int get_token_precedence() {
    if (!isascii(current_token)) {
        return -1;
    }
    int token_precedence = binop_precedence[current_token];
    if (token_precedence <= 0) {
        return -1;
    }
    return token_precedence;
}

// error helpers
std::unique_ptr<ExprAST> log_error(const char *str) {
    fprintf(stderr, "error : %s\n", str);
    return nullptr;
}
std::unique_ptr<PrototypeAST> log_prototype_error(const char *str) {
    log_error(str);
    return nullptr;
}

static std::unique_ptr<ExprAST> parse_expression();

// numberexpr ::= number
static std::unique_ptr<ExprAST> parse_number_expr() {
    auto result = std::make_unique<NumberExprAST>(number);
    get_next_token(); // consume number
    return std::move(result);
}

// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> parse_paren_expr() {
    get_next_token(); // consume open paren
    auto expression = parse_expression();
    if (!expression) {
        return nullptr;
    }
    if (current_token != ')') {
        return log_error("expected ')'");
    }
    get_next_token(); // consume closing paren
    return expression;
}

// identifierexpr ::= identifier | identifier '(' expression* ')'
static std::unique_ptr<ExprAST> parse_identifier_expr() {
    std::string id = identifier;
    get_next_token(); // consume identifier
    if (current_token != '(') {
        return std::make_unique<VariableExprAST>(id);
    }
    get_next_token(); // consume open paren
    std::vector<std::unique_ptr<ExprAST>> args;
    if (current_token != ')') {
        while (true) {
            if (auto arg = parse_expression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }
            if (current_token == ')') {
                break;
            }
            if (current_token != ',') {
                return log_error("expected ')' or ','");
            }
            get_next_token();
        }
    }
    get_next_token(); // consume closing paren
    return std::make_unique<CallExprAST>(id, std::move(args));
}

// primary ::= identifierexpr | numberexpr | parenexpr
static std::unique_ptr<ExprAST> parse_primary() {
    switch (current_token) {
    default:
        return log_error("expected expression");
    case tok_identifier:
        return parse_identifier_expr();
    case tok_number:
        return parse_number_expr();
    case '(':
        return parse_paren_expr();
    }
}

// binoprhs ::= ('+' primary)*
static std::unique_ptr<ExprAST> parse_binop_rhs(int expr_precedence, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        int token_precedence = get_token_precedence();
        if (token_precedence < expr_precedence) {
            return lhs;
        }
        int binop = current_token;
        get_next_token(); // consume binop
        // parse primary expression
        auto rhs = parse_primary();
        if (!rhs) {
            return nullptr;
        }
        int next_precedence = get_token_precedence();
        if (token_precedence < next_precedence) {
            rhs = parse_binop_rhs(token_precedence + 1, std::move(rhs));
            if (!rhs) {
                return nullptr;
            }
        }
        // merge
        lhs = std::make_unique<BinaryExprAST>(binop, std::move(lhs), std::move(rhs));
    }
}

// expression ::= primary binoprhs
static std::unique_ptr<ExprAST> parse_expression() {
    auto lhs = parse_primary();
    if (!lhs) {
        return nullptr;
    }
    return parse_binop_rhs(0, std::move(lhs));
}

// prototype ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> parse_prototype() {
    if (current_token != tok_identifier) {
        return log_prototype_error("expected function name in prototype");
    }
    std::string function_name = identifier;
    get_next_token(); // consume identifier
    if (current_token != '(') {
        return log_prototype_error("expected '(' in prototype");
    }
    // read list of arguments
    std::vector<std::string> args;
    while (get_next_token() == tok_identifier) {
        args.push_back(identifier);
    }
    if (current_token != ')') {
        return log_prototype_error("expected ')' in prototype");
    }
    get_next_token(); // consume closing paren
    return std::make_unique<PrototypeAST>(function_name, std::move(args));
}

// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> parse_definition() {
    get_next_token(); // consume 'def'
    auto prototype = parse_prototype();
    if (!prototype) {
        return nullptr;
    }
    if (auto expression = parse_expression()) {
        return std::make_unique<FunctionAST>(std::move(prototype), std::move(expression));
    }
    return nullptr;
}

// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> parse_toplevel_expr() {
    if (auto expression = parse_expression()) {
        // anonymous prototype
        auto prototype = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(prototype), std::move(expression));
    }
    return nullptr;
}

// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> parse_extern() {
    get_next_token(); // consume extern
    return parse_prototype();
}

// codegen
// ---------------------------------------
static std::unique_ptr<llvm::LLVMContext> context;
static std::unique_ptr<llvm::Module> mod;
static std::unique_ptr<llvm::IRBuilder<>> builder;
static std::map<std::string, llvm::Value *> named_values;

llvm::Value *log_error_value(const char *str) {
    log_error(str);
    return nullptr;
}

llvm::Value *NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*context, llvm::APFloat(num));
}

llvm::Value *VariableExprAST::codegen() {
    llvm::Value *named_value = named_values[identifier];
    if (!named_value) {
        return log_error_value("unknown variable name");
    }
    return named_value;
}

llvm::Value *BinaryExprAST::codegen() {
    llvm::Value *left = lhs->codegen();
    llvm::Value *right = rhs->codegen();
    if (!left || !right) {
        return nullptr;
    }
    switch (op) {
    case '+':
        return builder->CreateFAdd(left, right, "addtmp");
    case '-':
        return builder->CreateFSub(left, right, "subtmp");
    case '*':
        return builder->CreateFMul(left, right, "multmp");
    case '<':
        left = builder->CreateFCmpULT(left, right, "cmptmp");
        // convert bool int to double
        return builder->CreateUIToFP(left, llvm::Type::getDoubleTy(*context), "booltmp");
    default:
        return log_error_value("invalid binary operator");
    }
}

llvm::Value *CallExprAST::codegen() {
    // function name lookup
    llvm::Function *callee_function = mod->getFunction(callee);
    if (!callee_function) {
        return log_error_value("unknown function");
    }
    // argument size mismatch
    if (callee_function->arg_size() != args.size()) {
        return log_error_value("incorrect # arguments");
    }
    std::vector<llvm::Value *> argsv;
    for (unsigned i = 0, e = argsv.size(); i != e; ++i) {
        argsv.push_back(args[i]->codegen());
        if (!argsv.back()) {
            return nullptr;
        }
    }
    return builder->CreateCall(callee_function, argsv, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
    // function type : double(double, double)
    std::vector<llvm::Type *> doubles(args.size(), llvm::Type::getDoubleTy(*context));
    llvm::FunctionType *function_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), doubles, false);
    llvm::Function *function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name, mod.get());
    // arguments
    unsigned idx = 0;
    for (auto &arg : function->args()) {
        arg.setName(args[idx++]);
    }
    return function;
}

llvm::Function *FunctionAST::codegen() {
    // check for existing function (extern)
    llvm::Function *function = mod->getFunction(prototype->get_identifier());
    if (!function) {
        function = prototype->codegen();
    }
    if (!function) {
        return nullptr;
    }
    if (!function->empty()) {
        return (llvm::Function *) log_error_value("function cannot be redefined");
    }
    // create new block
    llvm::BasicBlock *basic_block = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(basic_block);
    // record
    named_values.clear();
    for (auto &arg : function->args()) {
        named_values[std::string(arg.getName())] = &arg;
    }
    if (llvm::Value *ret = body->codegen()) {
        // return
        builder->CreateRet(ret);
        // verify
        llvm::verifyFunction(*function); // check consistency
        return function;
    }
    // error
    function->eraseFromParent();
    return nullptr;
}

// top-level parser
// ---------------------------------------
static void handle_definition() {
    if (auto function_ast = parse_definition()) {
        if (auto *function_ir = function_ast->codegen()) {
            fprintf(stderr, "handle_definition\n");
            function_ir->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        get_next_token(); // skip for error recovery
    }
}

static void handle_extern() {
    if (auto prototype_ast = parse_extern()) {
        if (auto *function_ir = prototype_ast->codegen()) {
            fprintf(stderr, "handle_extern\n");
            function_ir->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        get_next_token(); // skip for error recovery
    }
}

static void handle_toplevel_expression() {
    if (auto function_ast = parse_toplevel_expr()) {
        if (auto *function_ir = function_ast->codegen()) {
            fprintf(stderr, "handle_toplevel_expression\n");
            function_ir->print(llvm::errs());
            fprintf(stderr, "\n");
            // remove anonymous expression
            function_ir->eraseFromParent();
        }
    } else {
        get_next_token(); // skip for error recovery
    }
}

// top ::= definition | external | expression | ';'
static void main_loop() {
    while (true) {
        fprintf(stderr, "ready> ");
        switch (current_token) {
        case tok_eof:
            return;
        case ';': // ignore
            get_next_token();
            break;
        case tok_def:
            handle_definition();
            break;
        case tok_extern:
            handle_extern();
            break;
        default:
            handle_toplevel_expression();
            break;
        }
    }
}

// main driver
// ---------------------------------------
// clang++ -g -O3 parser.cpp -o parser `llvm-config --cxxflags --ldflags --system-libs --libs core`; ./parser
int main() {
    // standard precedence for binary operators
    binop_precedence['<'] = 10;
    binop_precedence['+'] = 20;
    binop_precedence['-'] = 20;
    binop_precedence['*'] = 40;

    // first token
    fprintf(stderr, "ready> ");
    get_next_token();

    // init llvm
    context = std::make_unique<llvm::LLVMContext>();
    mod = std::make_unique<llvm::Module>("llvm codegen", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    // run main loop
    main_loop();

    // print generated llvm ir (ctrl+d to send EOF)
    mod->print(llvm::errs(), nullptr);
    // ready> ; ModuleID = 'llvm codegen'
    // source_filename = "llvm codegen"
}
