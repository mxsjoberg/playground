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
#include <iostream>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

////
//
// Lexer
//
////

enum Token {
  t_eof = -1,
  // commands
  t_def = -2,
  t_extern = -3,
  // primary
  t_identifier = -4,
  t_number = -5,
};

static std::string IdentifierStr; // t_identifier
static double NumVal; // t_number

// return the next token from input
static int get_t() {
  static int LastChar = ' ';
  // skip whitespace
  while (isspace(LastChar)) {
    LastChar = getchar();
  }
  // identifier : [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;
    while (isalnum((LastChar = getchar()))) {
      IdentifierStr += LastChar;
    }
    if (IdentifierStr == "def") {
      return t_def;
    }
    if (IdentifierStr == "extern") {
      return t_extern;
    }
    return t_identifier;
  }
  // number : [0-9.]+
  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr; // string to hold number
    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');
    NumVal = strtod(NumStr.c_str(), 0); // string to digit
    return t_number;
  }
  // skip comment
  if (LastChar == '#') {
    // skip until end of line
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
    if (LastChar != EOF) {
      return get_t();
    }
  }
  // check for end of file
  if (LastChar == EOF) {
    return t_eof;
  }
  // otherwise return unknown character (+, -, etc.)
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

////
//
// AST
// 
////

// anonymous namespace to prevent name clashes
namespace {
  // expression : base
  class ExprAST {
    public:
      virtual ~ExprAST() = default;
      virtual Value* codegen() = 0;
  };
  // expression : number (derived from base)
  class NumberExprAST : public ExprAST {
    double Val;
    
    public:
      NumberExprAST(double Val) : Val(Val) {}
      // generate code for number expression
      // Value is LLVM class for SSA
      Value* codegen() override;
  };
  // expression : variable
  class VariableExprAST : public ExprAST {
    // variable name
    std::string Name;
    
    public:
      VariableExprAST(const std::string &Name) : Name(Name) {}
      Value* codegen() override;
  };
  // expression : binary operator
  class BinaryExprAST : public ExprAST {
    // operator symbol (opcode)
    char Op;
    // pointer to left-hand and right-hand side of expression
    std::unique_ptr<ExprAST> LHS, RHS;
    
    public:
      // std::move() used to transfer ownership of pointers to member variables (std::unique_ptr objects cannot be copied)
      BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
      Value* codegen() override;
  };
  // expression : function calls
  class CallExprAST : public ExprAST {
    // function name
    std::string Callee;
    // list of argument expressions
    std::vector<std::unique_ptr<ExprAST>> Args;
    
    public:
      CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
      Value* codegen() override;
  };
  // prototype : name and arguments of function
  class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
    
    public:
      PrototypeAST(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
      Function* codegen();

      const std::string &getName() const { return Name; }
  };
  // function : function definition
  class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
    
    public:
      FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
      Function* codegen();
  };
}

///
//
// Parser
//
///

static int CurTok;
static int getNextToken() { return CurTok = get_t(); }

static std::unique_ptr<ExprAST> ParseExpression();
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS);

// operator precedence
static std::map<char, int> BinopPrecedence;

// get precedence of current token
static int GetTokPrecedence() {
  if (!isascii(CurTok)) {
    return -1;
  }
  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0) {
    return -1;
  }
  return TokPrec;
}

// helper function to log error
static std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

// numberexpr :== number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken(); // consume the number
  return std::move(Result);
}

// parenexpr :== '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // consume '('
  auto V = ParseExpression();
  if (!V) {
    return nullptr;
  }
  if (CurTok != ')') {
    return LogError("expected ')'");
  }
  getNextToken(); // consume ')'
  return V;
}

// identifierexpr :== identifier | identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = IdentifierStr;
  getNextToken(); // consume identifier
  if (CurTok != '(') {
    return std::make_unique<VariableExprAST>(IdName);
  }
  // call
  getNextToken(); // consume '('
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression()) {
        Args.push_back(std::move(Arg));
      } else {
        return nullptr;
      }
      if (CurTok == ')') { break; }
      if (CurTok != ',') {
        return LogError("expected ')' or ',' in argument list");
      }
      getNextToken();
    }
  }
  getNextToken(); // consume ')'

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

// primary :== identifierexpr | numberexpr | parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
    default:
      return LogError("unknown token when expecting an expression");
    case t_identifier:
      return ParseIdentifierExpr();
    case t_number:
      return ParseNumberExpr();
    case '(':
      return ParseParenExpr();
  }
}

// binoprhs :== ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
  // if binop, find precedence
  while (true) {
    int TokPrec = GetTokPrecedence();
    // lower precedence than current expression
    if (TokPrec < ExprPrec) {
      return LHS;
    }
    int BinOp = CurTok;
    getNextToken(); // consume binop
    // parse the primary expression after the binary operator
    auto RHS = ParsePrimary();
    if (!RHS) {
      return nullptr;
    }
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS) {
        return nullptr;
      }
    }
    // merge
    LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  }
}

// expression :== primary binoprhs
static std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS) {
    return nullptr;
  }
  return ParseBinOpRHS(0, std::move(LHS));
}

// prototype :== identifier '(' identifier* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  if (CurTok != t_identifier) {
    return LogErrorP("expected function name in prototype");
  }
  std::string FnName = IdentifierStr;
  getNextToken(); // consume identifier
  if (CurTok != '(') {
    return LogErrorP("expected '(' in prototype");
  }
  // read the list of argument names
  std::vector<std::string> ArgNames;
  while (getNextToken() == t_identifier) {
    ArgNames.push_back(IdentifierStr);
  }
  if (CurTok != ')') {
    return LogErrorP("expected ')' in prototype");
  }
  // success
  getNextToken(); // consume ')'
  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

// definition :== 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken(); // consume 'def'
  auto Proto = ParsePrototype();
  if (!Proto) {
    return nullptr;
  }
  if (auto E = ParseExpression()) {
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // make an anonymous proto
    auto Proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken(); // consume 'extern'
  return ParsePrototype();
}

///
//
// LLVM Code Generation
//
///

// LLVMContext is the top-level container for all LLVM global data
static std::unique_ptr<LLVMContext> TheContext;
// IRBuilder is a helper class that provides a nice interface for creating instructions
static std::unique_ptr<IRBuilder<>> Builder;
// Module is the top-level container for all LLVM code (i.e. symbol table)
static std::unique_ptr<Module> TheModule;
// keep track of values defined in the current scope
static std::map<std::string, Value*> NamedValues;

// helper function to log errors
Value* LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}

Value* NumberExprAST::codegen() {
  // constant floating point, AP -> arbitrary precision
  return ConstantFP::get(*TheContext, APFloat(Val));
}

Value* VariableExprAST::codegen() {
  // look this variable up in the function
  Value* V = NamedValues[Name];
  if (!V) {
    LogErrorV("unknown variable name");
  }
  return V;
}

Value* BinaryExprAST::codegen() {
  Value* L = LHS->codegen();
  Value* R = RHS->codegen();
  if (!L || !R) {
    return nullptr;
  }
  switch (Op) {
    case '+':
      return Builder->CreateFAdd(L, R, "addtmp");
    case '-':
      return Builder->CreateFSub(L, R, "subtmp");
    case '*':
      return Builder->CreateFMul(L, R, "multmp");
    case '<':
      L = Builder->CreateFCmpULT(L, R, "cmptmp");
      // convert bool to double
      return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
    default:
      return LogErrorV("invalid binary operator");
  }
}

Value* CallExprAST::codegen() {
  // look up the name in the global module table
  Function* CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF) {
    return LogErrorV("unknown function referenced");
  }
  // if argument mismatch
  if (CalleeF->arg_size() != Args.size()) {
    return LogErrorV("incorrect number arguments passed");
  }
  std::vector<Value*> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back()) {
      return nullptr;
    }
  }
  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

Function* PrototypeAST::codegen() {
  // make function type -> double(double, double)
  std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
  FunctionType* FT = FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);
  // ExternalLinkage means the function can be accessed from other modules
  Function* F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());
  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(Args[Idx++]);
  }
  return F;
}

Function* FunctionAST::codegen() {
  // check if function already exists
  Function* TheFunction = TheModule->getFunction(Proto->getName());
  if (!TheFunction) {
    TheFunction = Proto->codegen();
  }
  if (!TheFunction) {
    return nullptr;
  }
  if (!TheFunction->empty()) {
    return (Function*)LogErrorV("function cannot be redefined");
  }
  // create new basic block to start insertion into
  // https://en.wikipedia.org/wiki/Basic_block
  BasicBlock* BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);
  // record function arguments in NamedValues map
  NamedValues.clear();
  for (auto &Arg : TheFunction->args()) {
    NamedValues[std::string(Arg.getName())] = &Arg;
  }
  if (Value* RetVal = Body->codegen()) {
    // finish off the function
    Builder->CreateRet(RetVal);
    // validate generated code, checking for consistency
    verifyFunction(*TheFunction);
    return TheFunction;
  }
  TheFunction->eraseFromParent();
  return nullptr;
}

///
//
// Top-Level parsing (JIT)
//
///

static void InitModule() {
  // create new contet and module
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("JIT", *TheContext);
  // builder for module
  Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

static void HandleDefinition() {
  if (auto FnAST = ParseDefinition()) {
    // fprintf(stderr, "parsed function definition\n");
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "read function definition:");
      FnIR->print(errs());
      fprintf(stderr, "\n");
    }
  } else {
    // skip
    getNextToken();
  }
}

static void HandleExtern() {
  if (auto ProtoAST = ParseExtern()) {
    // fprintf(stderr, "parsed extern\n");
    if (auto *FnIR = ProtoAST->codegen()) {
      fprintf(stderr, "read extern:");
      FnIR->print(errs());
      fprintf(stderr, "\n");
    }
  } else {
    // skip
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  if (auto FnAST = ParseTopLevelExpr()) {
    // fprintf(stderr, "parsed top-level expr\n");
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "read top-level expr:");
      FnIR->print(errs());
      fprintf(stderr, "\n");
      // remove anonymous expression
      FnIR->eraseFromParent();
    }
  } else {
    // skip
    getNextToken();
  }
}

// top ::= definition | external | expression | ';'
static void MainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (CurTok) {
      case t_eof:
        return;
      case ';':
        // skip
        getNextToken();
        break;
      case t_def:
        HandleDefinition();
        break;
      case t_extern:
        HandleExtern();
        break;
      default:
        HandleTopLevelExpression();
        break;
    }
  }
}

////
//
// Driver
//
////

int main() {
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40; // highest
  // first token
  fprintf(stderr, "ready> ");
  getNextToken();
  // init module (holds all code)
  InitModule();
  // run
  MainLoop();
  // output generated code
  TheModule->print(errs(), nullptr);
}
