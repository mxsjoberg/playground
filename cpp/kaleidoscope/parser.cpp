// https://releases.llvm.org/14.0.0/docs/tutorial/MyFirstLanguageFrontend/LangImpl06.html

/*
ready> 4+5;

    define double @__anon_expr() {
    entry:
      ret double 9.000000e+00
    }
*/

/*
ready> def foo(a b) a*a + 2*a*b + b*b;

    define double @foo(double %a, double %b) {
    entry:
      %multmp = fmul double %a, %a
      %multmp1 = fmul double 2.000000e+00, %a
      %multmp2 = fmul double %multmp1, %b
      %addtmp = fadd double %multmp, %multmp2
      %multmp3 = fmul double %b, %b
      %addtmp4 = fadd double %addtmp, %multmp3
      ret double %addtmp4
    }
*/

/*
ready> def bar(a) foo(a, 4.0) + bar(31337);

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
    
    declare double @cos(double)

ready> cos(1.234);
    
    define double @__anon_expr() {
    entry:
      %calltmp = call double @cos()
      ret double %calltmp
    }
*/

/*
ready> extern foo();
    
    declare double @foo()
    
ready> extern bar();
    
    declare double @bar()
    
ready> def baz(x) if x then foo() else bar();

    define double @baz(double %x) {
    entry:
      %ifcond = fcmp one double %x, 0.000000e+00
      br i1 %ifcond, label %then, label %else

    then:                                             ; preds = %entry
      %calltmp = call double @foo()
      br label %ifcont

    else:                                             ; preds = %entry
      %calltmp1 = call double @bar()
      br label %ifcont

    ifcont:                                           ; preds = %else, %then
      %iftmp = phi double [ %calltmp, %then ], [ %calltmp1, %else ]
      ret double %iftmp
    }
*/

/*
ready> extern putchard(char);
    
    declare double @putchard(double)

ready> def printstar(n) for i = 1, i < n, 1.0 in putchard(42);

    define double @printstar(double %n) {
    entry:
      br label %loop

    loop:                                             ; preds = %loop, %entry
      %i = phi double [ 1.000000e+00, %entry ], [ %nextvar, %loop ]
      %calltmp = call double @putchard()
      %nextvar = fadd double %i, 1.000000e+00
      %cmptmp = fcmp ult double %i, %n
      %booltmp = uitofp i1 %cmptmp to double
      %loopcond = fcmp one double %booltmp, 0.000000e+00
      br i1 %loopcond, label %loop, label %afterloop

    afterloop:                                        ; preds = %loop
      ret double 0.000000e+00
    }

ready> printstar(10);

    define double @__anon_expr() {
    entry:
      %calltmp = call double @printstar()
      ret double %calltmp
    }
*/

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
// llvm
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
// #include "llvm/Passes/PassBuilder.h"
// #include "llvm/Passes/StandardInstrumentations.h"
// #include "llvm/Support/TargetSelect.h"
// #include "llvm/Target/TargetMachine.h"
// #include "llvm/Transforms/InstCombine/InstCombine.h"
// #include "llvm/Transforms/Scalar.h"
// #include "llvm/Transforms/Scalar/GVN.h"
// #include "llvm/Transforms/Scalar/Reassociate.h"
// #include "llvm/Transforms/Scalar/SimplifyCFG.h"

// #include "KaleidoscopeJIT.h"

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
    // control
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,
    tok_for = -9,
    tok_in = -10,
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
        if (identifier == "def") { return tok_def; }
        if (identifier == "extern") { return tok_extern; }
        if (identifier == "if") { return tok_if; }
        if (identifier == "then") { return tok_then; }
        if (identifier == "else") { return tok_else; }
        if (identifier == "for") { return tok_for; }
        if (identifier == "in") { return tok_in; }

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
namespace { // start anonymous namespace

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

class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> cond, then, else_;
public:
    IfExprAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> then, std::unique_ptr<ExprAST> else_)
        : cond(std::move(cond)), then(std::move(then)), else_(std::move(else_)) {}
    // llvm
    llvm::Value *codegen() override;
}; 

class ForExprAST : public ExprAST {
    std::string var_name;
    std::unique_ptr<ExprAST> start, end, step, body;
public:
    ForExprAST(const std::string &var_name, std::unique_ptr<ExprAST> start, std::unique_ptr<ExprAST> end, std::unique_ptr<ExprAST> step, std::unique_ptr<ExprAST> body)
        : var_name(var_name), start(std::move(start)), end(std::move(end)), step(std::move(step)), body(std::move(body)) {}
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

// ifexpr ::= 'if' expression 'then' expression 'else' expression
static std::unique_ptr<ExprAST> parse_if_expr() {
    get_next_token(); // consume if
    auto cond = parse_expression();
    if (!cond) {
        return nullptr;
    }
    if (current_token != tok_then) {
        return log_error("expected then");
    }
    get_next_token(); // consume then
    auto then = parse_expression();
    if (!then) {
        return nullptr;
    }
    if (current_token != tok_else) {
        return log_error("expected else");
    }
    get_next_token(); // consume else
    auto else_ = parse_expression();
    if (!else_) {
        return nullptr;
    }
    return std::make_unique<IfExprAST>(std::move(cond), std::move(then), std::move(else_));
}

// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
static std::unique_ptr<ExprAST> parse_for_expr() {
    get_next_token(); // consume for
    if (current_token != tok_identifier) {
        return log_error("expected identifier after for");
    }
    std::string id = identifier;
    get_next_token(); // consume identifier
    if (current_token != '=') {
        return log_error("expected '=' after for");
    }
    get_next_token(); // consume '='
    auto start = parse_expression();
    if (!start) {
        return nullptr;
    }
    if (current_token != ',') {
        return log_error("expected ',' after for start value");
    }
    get_next_token(); // consume ','
    auto end = parse_expression();
    if (!end) {
        return nullptr;
    }
    // optional step value
    std::unique_ptr<ExprAST> step;
    if (current_token == ',') {
        get_next_token(); // consume ','
        step = parse_expression();
        if (!step) {
            return nullptr;
        }
    }
    if (current_token != tok_in) {
        return log_error("expected 'in' after for");
    }
    get_next_token(); // consume in
    auto body = parse_expression();
    if (!body) {
        return nullptr;
    }
    return std::make_unique<ForExprAST>(id, std::move(start), std::move(end), std::move(step), std::move(body));

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
    case tok_if:
        return parse_if_expr();
    case tok_for:
        return parse_for_expr();
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
static std::map<std::string, std::unique_ptr<PrototypeAST>> function_prototypes;
// passes
// static std::unique_ptr<llvm::FunctionPassManager> function_pass_manager;
// static std::unique_ptr<llvm::LoopAnalysisManager> loop_analysis_manager;
// static std::unique_ptr<llvm::FunctionAnalysisManager> function_analysis_manager;
// static std::unique_ptr<llvm::CGSCCAnalysisManager> cgscc_analysis_manager;
// static std::unique_ptr<llvm::ModuleAnalysisManager> module_analysis_manager;
// static std::unique_ptr<llvm::PassInstrumentationCallbacks> pass_instrumentation_callbacks;
// static std::unique_ptr<llvm::StandardInstrumentations> standard_instrumentations;
// jit
// static std::unique_ptr<llvm::orc::KaleidoscopeJIT> jit;

llvm::Value *log_error_value(const char *str) {
    log_error(str);
    return nullptr;
}

// get function helper
llvm::Function *get_function(const std::string &name) {
    // check if function is already in module
    if (auto *function = mod->getFunction(name)) {
        return function;
    }
    // check if function is in prototype
    auto prototype = function_prototypes.find(name);
    if (prototype != function_prototypes.end()) {
        return prototype->second->codegen();
    }
    // not found
    return nullptr;
}

llvm::Value *NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*context, llvm::APFloat(num));
}

llvm::Value *VariableExprAST::codegen() {
    llvm::Value *named_value = named_values[id];
    if (!named_value) {
        return log_error_value("unknown variable id");
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

llvm::Value *IfExprAST::codegen() {
    llvm::Value *cond_value = cond->codegen();
    if (!cond_value) {
        return nullptr;
    }
    // convert condition to bool
    cond_value = builder->CreateFCmpONE(cond_value, llvm::ConstantFP::get(*context, llvm::APFloat(0.0)), "ifcond");
    // create blocks
    llvm::Function *function = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *then_block = llvm::BasicBlock::Create(*context, "then", function);
    llvm::BasicBlock *else_block = llvm::BasicBlock::Create(*context, "else"); // not added to function
    llvm::BasicBlock *merge_block = llvm::BasicBlock::Create(*context, "ifcont"); // not added to function
    builder->CreateCondBr(cond_value, then_block, else_block);
    // emit then block
    builder->SetInsertPoint(then_block);
    llvm::Value *then_value = then->codegen();
    if (!then_value) {
        return nullptr;
    }
    builder->CreateBr(merge_block);
    then_block = builder->GetInsertBlock();
    // emit else block
    function->getBasicBlockList().push_back(else_block); // add to function
    builder->SetInsertPoint(else_block);
    llvm::Value *else_value = else_->codegen();
    if (!else_value) {
        return nullptr;
    }
    builder->CreateBr(merge_block);
    else_block = builder->GetInsertBlock();
    // emit merge block
    function->getBasicBlockList().push_back(merge_block); // add to function
    builder->SetInsertPoint(merge_block);
    // llvm ssa
    llvm::PHINode *phi_node = builder->CreatePHI(llvm::Type::getDoubleTy(*context), 2, "iftmp");
    phi_node->addIncoming(then_value, then_block);
    phi_node->addIncoming(else_value, else_block);
    return phi_node;
}

llvm::Value *ForExprAST::codegen() {
    // emit start code
    llvm::Value *start_value = start->codegen();
    if (!start_value) {
        return nullptr;
    }
    // create basic block
    llvm::Function *function = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *preheader_block = builder->GetInsertBlock();
    llvm::BasicBlock *loop_block = llvm::BasicBlock::Create(*context, "loop", function);
    // insert branch
    builder->CreateBr(loop_block);
    // emit loop block
    builder->SetInsertPoint(loop_block);
    // create phi node
    llvm::PHINode *variable = builder->CreatePHI(llvm::Type::getDoubleTy(*context), 2, var_name.c_str());
    variable->addIncoming(start_value, preheader_block);
    // save old variable
    llvm::Value *old_value = named_values[var_name];
    named_values[var_name] = variable;
    // emit body
    if (!body->codegen()) {
        return nullptr;
    }
    // emit step
    llvm::Value *step_value = nullptr;
    if (step) {
        step_value = step->codegen();
        if (!step_value) {
            return nullptr;
        }
    } else {
        step_value = llvm::ConstantFP::get(*context, llvm::APFloat(1.0));
    }
    llvm::Value *next_value = builder->CreateFAdd(variable, step_value, "nextvar");
    // end condition
    llvm::Value *end_condition = end->codegen();
    if (!end_condition) {
        return nullptr;
    }
    // convert condition to bool
    end_condition = builder->CreateFCmpONE(end_condition, llvm::ConstantFP::get(*context, llvm::APFloat(0.0)), "loopcond");
    // create loop block
    llvm::BasicBlock *loop_end_block = builder->GetInsertBlock();
    llvm::BasicBlock *after_block = llvm::BasicBlock::Create(*context, "afterloop", function);
    // insert branch
    builder->CreateCondBr(end_condition, loop_block, after_block);
    // emit after block
    builder->SetInsertPoint(after_block);
    // add incoming
    variable->addIncoming(next_value, loop_end_block);
    // restore old variable
    if (old_value) {
        named_values[var_name] = old_value;
    } else {
        named_values.erase(var_name);
    }
    // return null
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context));
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
    auto &p = *prototype; // transfer ownership
    function_prototypes[prototype->get_identifier()] = std::move(prototype);
    // check for existing function (extern)
    llvm::Function *function = get_function(p.get_identifier());
    if (!function) {
        return nullptr;
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
        // optimize
        // function_pass_manager->run(*function, *function_analysis_manager);
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
            // fprintf(stderr, "handle_definition\n");
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
            // fprintf(stderr, "handle_extern\n");
            function_ir->print(llvm::errs());
            fprintf(stderr, "\n");
            function_prototypes[prototype_ast->get_identifier()] = std::move(prototype_ast);
        }
    } else {
        get_next_token(); // skip for error recovery
    }
}

static void handle_toplevel_expression() {
    if (auto function_ast = parse_toplevel_expr()) {
        if (auto *function_ir = function_ast->codegen()) {
            // fprintf(stderr, "handle_toplevel_expression\n");
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
        // fprintf(stderr, "ready> ");
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

    // std::string program = R"(4+5;)";
    // use program as cin for getchar
    // std::stringbuf program_buffer(program);
    // std::streambuf *cinbuf = std::cin.rdbuf();
    // std::cin.rdbuf(&program_buffer);

    // first token
    fprintf(stderr, "ready> ");
    get_next_token();

    // init llvm
    context = std::make_unique<llvm::LLVMContext>();
    mod = std::make_unique<llvm::Module>("llvm codegen", *context);
    // data layout
    // mod->setDataLayout(jit->getDataLayout());
    // ir builder
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    // llvm optimizations
    // function_pass_manager = std::make_unique<llvm::FunctionPassManager>();
    // loop_analysis_manager = std::make_unique<llvm::LoopAnalysisManager>();
    // function_analysis_manager = std::make_unique<llvm::FunctionAnalysisManager>();
    // cgscc_analysis_manager = std::make_unique<llvm::CGSCCAnalysisManager>();
    // module_analysis_manager = std::make_unique<llvm::ModuleAnalysisManager>();
    // pass_instrumentation_callbacks = std::make_unique<llvm::PassInstrumentationCallbacks>();
    // standard_instrumentations = std::make_unique<llvm::StandardInstrumentations>(*context, true); // true for debugging
    
    // standard_instrumentations->registerCallbacks(*pass_instrumentation_callbacks, module_analysis_manager.get());

    // peephole optimizations
    // function_pass_manager->addPass(llvm::Passes::createFunctionToLoopPassAdaptor(llvm::Passes::createInstructionCombiningPass()));
    // reassociate expressions
    // function_pass_manager->addPass(llvm::Passes::createFunctionToLoopPassAdaptor(llvm::Passes::createReassociatePass()));
    // eliminate common subexpressions
    // function_pass_manager->addPass(llvm::Passes::createFunctionToLoopPassAdaptor(llvm::Passes::createNewGVNPass()));
    // simplify control flow graph
    // function_pass_manager->addPass(llvm::Passes::createFunctionToLoopPassAdaptor(llvm::Passes::createCFGSimplificationPass()));

    // register analysis
    // llvm::PassBuilder pass_builder;
    // pass_builder.registerModuleAnalyses(*module_analysis_manager);
    // pass_builder.registerFunctionAnalyses(*function_analysis_manager);
    // pass_builder.crossRegisterProxies(*loop_analysis_manager, *function_analysis_manager, *cgscc_analysis_manager, *module_analysis_manager);


    // run main loop
    main_loop();

    // print generated llvm ir (ctrl+d to send EOF)
    mod->print(llvm::errs(), nullptr);
    // ready> ; ModuleID = 'llvm codegen'
    // source_filename = "llvm codegen"
}
