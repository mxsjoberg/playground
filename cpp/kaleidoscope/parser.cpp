/*
    https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html
    
    # Compute the x'th fibonacci number.
    def fib(x)
        if x < 3 then
            1
        else
            fib(x-1)+fib(x-2)
    # This expression will compute the 40th number.
    fib(40)
*/

// clang++ -g -O3 parser.cpp -o parser `llvm-config --cxxflags`; ./parser

#import <cctype>
#import <cstdio>
#import <cstdlib>
#import <map>
#import <memory>
#import <string>
#import <utility>
#import <vector>

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
    // identifier: [a-zA-Z][a-zA-Z0-9]*
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

        tmp_number = strtod(tmp_number.c_str(), 0);
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

class ExprAST {
    public:
        virtual ~ExprAST() = default; // virtual destructor
};

class NumberExprAST: public ExprAST {
    double number;
    public:
        NumberExprAST(double number)
            : number(number) {}
};

class VariableExprAST : public ExprAST {
    std::string identifier;
    public:
        VariableExprAST(const std::string &identifier)
            : identifier(identifier) {}
};

class BinaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> lhs, rhs;
    public:
        BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

class CallExprAST: public ExprAST {
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
    public:
        CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args)
            : callee(callee), args(std::move(args)) {}
};

// functions
class PrototypeAST {
    std::string identifier;
    std::vector<std::string> args;
    public:
        PrototypeAST(const std::string &identifier, std::vector<std::string> args)
            : identifier(identifier), args(std::move(args)) {}
        // getter
        const std::string &get_identifier() const { return identifier; }
};

class FunctionAST {
    std::unique_ptr<PrototypeAST> prototype;
    std::unique_ptr<ExprAST> body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> prototype, std::unique_ptr<ExprAST> body)
            : prototype(std::move(prototype)), body(std::move(body)) {}
};

// parser
// ---------------------------------------

static int current_token;
static int get_next_token() {
    return current_token = next_token();
}

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
static std::unique_ptr<ExprAST> parse_primary();
static std::unique_ptr<ExprAST> parse_binop_rhs(int expr_precedence, std::unique_ptr<ExprAST> lhs);

// numberexpr ::= number
static std::unique_ptr<ExprAST> parse_number_expr() {
    auto result = std::make_unique<NumberExprAST>(number);
    get_next_token(); // consume number
    return std::move(result);
}

// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> parse_paren_expr() {
    get_next_token(); // consume open paren
    auto v = parse_expression();
    if (!v) {
        return nullptr;
    }
    if (current_token != ')') {
        return log_error("expected ')'");
    }
    get_next_token(); // consume closing paren
    return v;
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

// expression ::= primary binoprhs
static std::unique_ptr<ExprAST> parse_expression() {
    auto lhs = parse_primary();
    if (!lhs) {
        return nullptr;
    }
    return parse_binop_rhs(0, std::move(lhs));
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

// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> parse_extern() {
    get_next_token(); // consume extern
    return parse_prototype();
}

// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> parse_toplevel_expr() {
    if (auto expression = parse_expression()) {
        // anonymous prototype
        auto prototype = std::make_unique<PrototypeAST>("", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(prototype), std::move(expression));
    }
    return nullptr;
}

// top-level parser
// ---------------------------------------

static void handle_definition() {
    if (parse_definition()) {
        fprintf(stderr, "function definition\n");
    } else {
        get_next_token(); // skip for error recovery
    }
}

static void handle_extern() {
    if (parse_extern()) {
        fprintf(stderr, "extern\n");
    } else {
        get_next_token(); // skip for error recovery
    }
}

static void handle_toplevel_expression() {
    if (parse_toplevel_expr()) {
        fprintf(stderr, "top-level expression\n");
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

int main() {
    // standard precedence for binary operators
    binop_precedence['<'] = 10;
    binop_precedence['+'] = 20;
    binop_precedence['-'] = 20;
    binop_precedence['*'] = 40;

    // first token
    fprintf(stderr, "ready> ");
    get_next_token();

    // run main loop
    main_loop();
}
