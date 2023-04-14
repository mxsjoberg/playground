# import sys
# print(sys.version)
from prettytable import PrettyTable

INTEGER         = 'INTEGER'
REAL            = 'REAL'
INTEGER_CONST   = 'INTEGER_CONST'
REAL_CONST      = 'REAL_CONST'
PLUS            = 'PLUS'
MINUS           = 'MINUS'
MULTIPLY        = 'MULTIPLY'
INTEGER_DIV     = 'INTEGER_DIV'
FLOAT_DIV       = 'FLOAT_DIV'
LPAR            = 'LPAR'
RPAR            = 'RPAR'
ID              = 'ID'
ASSIGN          = 'ASSIGN'
BEGIN           = 'BEGIN'
END             = 'END'
SEMI            = 'SEMI'
DOT             = 'DOT'
PROGRAM         = 'PROGRAM'
PROCEDURE       = 'PROCEDURE'
VAR             = 'VAR'
COLON           = 'COLON'
COMMA           = 'COMMA'
EOF             = 'EOF'

class Token(object):
    def __init__(self, type_, value):
        self.type_ = type_
        self.value = value

    # string representation -> Token(PLUS, '+')
    # def __str__(self):
    #     return 'Token({type_}, {value})'.format(
    #         type_ = self.type_,
    #         value = repr(self.value)
    #     )

    # string representation of token -> Token(PLUS, '+')
    def __repr__(self):
        # return self.__str__()
        return 'Token({type_}, {value})'.format(
            type_ = self.type_,
            value = str(self.value)
        )

RESERVED_KEYWORDS = {
    'PROGRAM'   : Token('PROGRAM', 'PROGRAM'),
    'PROCEDURE' : Token('PROCEDURE', 'PROCEDURE'),
    'VAR'       : Token('VAR', 'VAR'),
    'DIV'       : Token('INTEGER_DIV', 'DIV'),
    'INTEGER'   : Token('INTEGER', 'INTEGER'),
    'REAL'      : Token('REAL', 'REAL'),
    'BEGIN'     : Token('BEGIN', 'BEGIN'),
    'END'       : Token('END', 'END')
}

# ---------------------------------------------------
# lexer -> Token
# ---------------------------------------------------
class Lexer(object):
    def __init__(self, text):
        # text input
        self.text = text
        # index in text input
        self.pos = 0
        # current character
        self.current_char = self.text[self.pos]

    def error(self):
        # invalid character
        raise Exception('Error: Invalid character \'{}\''.format(self.current_char))

    # increment index in text input and set current character
    def increment(self):
        self.pos += 1
        # no more input?
        if (self.pos > len(self.text) - 1):
            self.current_char = None
        # otherwise
        else:
            self.current_char = self.text[self.pos]

    # lookahead -> char in next pos or None
    def peek(self):
        peek_pos = self.pos + 1
        # no more input?
        if (peek_pos > len(self.text) - 1):
            return None
        # otherwise
        else:
            return self.text[peek_pos]

    # skip whitespace
    def skip_whitespace(self):
        while (self.current_char is not None and self.current_char.isspace()):
            self.increment()

    # skip comment
    def skip_comment(self):
        while (self.current_char != '}'):
            self.increment()
        # closing curly brace
        self.increment()

    # integer or float -> Token(INTEGER_CONST | REAL_CONST)
    def number(self):
        result = ''
        # multi-digit numbers
        while (self.current_char is not None and self.current_char.isdigit()):
            result += self.current_char
            self.increment()
        # real numbers
        if (self.current_char == '.'):
            result += self.current_char
            self.increment()
            while (self.current_char is not None and self.current_char.isdigit()):
                result += self.current_char
                self.increment()
            token = Token('REAL_CONST', float(result))
        else:
            token = Token('INTEGER_CONST', int(result))

        return token

    # id and reserved keywords -> Token(ID | RESERVED_KEYWORD)
    def _id(self):
        result = ''
        while (self.current_char is not None and self.current_char.isalnum()):
            result += self.current_char
            self.increment()

        token = RESERVED_KEYWORDS.get(result.upper(), Token(ID, result.lower()))
        return token

    # tokenizer : break text input into tokens
    def get_next_token(self):
        while (self.current_char is not None):
            # whitespace
            if (self.current_char.isspace()):
                self.skip_whitespace()
                continue
            # comment
            if (self.current_char == '{'):
                self.increment()
                self.skip_comment()
                continue
            # id or reserved keywords
            if (self.current_char.isalpha()):
                return self._id()
            # number
            if (self.current_char.isdigit()):
                return self.number()
            # ASSIGN
            if (self.current_char == ':' and self.peek() == '='):
                self.increment()
                self.increment()
                return Token(ASSIGN, ':=')
            # SEMI
            if (self.current_char == ';'):
                self.increment()
                return Token(SEMI, ';')
            # COLON
            if (self.current_char == ':'):
                self.increment()
                return Token(COLON, ':')
            # COMMA
            if (self.current_char == ','):
                self.increment()
                return Token(COMMA, ',')
            # PLUS
            if (self.current_char == '+'):
                self.increment()
                return Token(PLUS, '+')
            # MINUS
            if (self.current_char == '-'):
                self.increment()
                return Token(MINUS, '-')
            # MULTIPLY
            if (self.current_char == '*'):
                self.increment()
                return Token(MULTIPLY, '*')
            # FLOAT_DIV
            if (self.current_char == '/'):
                self.increment()
                return Token(FLOAT_DIV, '/')
            # LPAR
            if (self.current_char == '('):
                self.increment()
                return Token(LPAR, '(')
            # RPAR
            if (self.current_char == ')'):
                self.increment()
                return Token(RPAR, ')')
            # DOT
            if (self.current_char == '.'):
                self.increment()
                return Token(DOT, '.')

            self.error()

        return Token(EOF, None)

# ---------------------------------------------------
# parser (syntax analysis) -> AST
# ---------------------------------------------------
class AST(object):
    pass

class Symbol(object):
    def __init__(self, name, type_=None):
        self.name = name
        self.type_ = type_
        # self.category = category

class BuiltinTypeSymbol(Symbol):
    def __init__(self, name):
        super().__init__(name)

    # def __str__(self):
    #     return self.name

    def __repr__(self):
        '''
            $ from main import BuiltinTypeSymbol
            $ int_ = BuiltinTypeSymbol('INTEGER')
            $ int_
            INTEGER
        '''
        # return self.name
        return "<{class_name}(name='{name}')>".format(
            class_name = self.__class__.__name__,
            name = self.name)

class VarSymbol(Symbol):
    def __init__(self, name, type_):
        super().__init__(name, type_)

    def __repr__(self):
        '''
            $ from main import BuiltinTypeSymbol, VarSymbol
            $ int_type = BuiltinTypeSymbol('INTEGER')
            $ var_x_symbol = VarSymbol('x', int_type)
            $ var_x_symbol
            <x:INTEGER>
        '''
        return "<{class_name}(name='{name}', type='{type_}'>".format(
            class_name = self.__class__.__name__,
            name = self.name,
            type_ = self.type_
        )

class Compound(AST):
    def __init__(self):
        self.children = []

class Assign(AST):
    def __init__(self, left, operator, right):
        self.left = left
        self.token = operator
        self.operator = operator
        self.right = right

class Variable(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value

class Empty(AST):

    pass

class BinaryOperator(AST):
    def __init__(self, left, operator, right):
        self.left = left
        self.token = operator
        self.operator = operator
        self.right = right

class UnaryOperator(AST):
    def __init__(self, operator, expr):
        self.token = operator
        self.operator = operator
        self.expr = expr

class Number(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value

class Program(AST):
    def __init__(self, name, block):
        self.name = name
        self.block = block

class Block(AST):
    def __init__(self, declarations, compound_statement):
        self.declarations = declarations
        self.compound_statement = compound_statement

class ProcedureDeclaration(AST):
    def __init__(self, procedure_name, block_node):
        self.procedure_name = procedure_name
        self.block_node = block_node

class VariableDeclaration(AST):
    def __init__(self, variable_node, type_node):
        self.variable_node = variable_node
        self.type_node = type_node

class Type(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value

class Parser(object):
    def __init__(self, lexer):
        self.lexer = lexer
        # current token
        self.current_token = self.lexer.get_next_token()

    def error(self):
        # invalid syntax
        raise Exception('Error: Invalid syntax')

    # eat current token
    def eat(self, token_type):
        # match with passed token type?
        if (self.current_token.type_ == token_type):
            self.current_token = self.lexer.get_next_token()
        # otherwise
        else:
            self.error()

    # program : PROGRAM variable SEMI block DOT
    def program(self):
        self.eat(PROGRAM)
        variable_node = self.variable()
        # program_name = variable_node.value
        self.eat(SEMI)
        block_node = self.block()
        # program_node = Program(program_name, block_node)
        self.eat(DOT)

        program_name = variable_node.value

        node = Program(program_name, block_node)
        return node

    # block : declarations compound_statement
    def block(self):
        declaration_nodes = self.declarations()
        compound_statement_node = self.compound_statement()

        node = Block(declaration_nodes, compound_statement_node)
        return node

    # declarations : VAR (variable_declaration SEMI)+ | (PROCEDURE ID SEMI block SEMI)* | empty
    def declarations(self):
        declarations = []

        # VAR (variable_declaration SEMI)+
        if self.current_token.type_ == VAR:
            self.eat(VAR)
            while self.current_token.type_ == ID:
                variable_declaration = self.variable_declaration()
                declarations.extend(variable_declaration)
                self.eat(SEMI)

        # (PROCEDURE ID SEMI block SEMI)*
        while self.current_token.type_ == PROCEDURE:
            self.eat(PROCEDURE)
            procedure_name = self.current_token.value
            self.eat(ID)
            self.eat(SEMI)
            block_node = self.block()
            procedure_declaration = ProcedureDeclaration(procedure_name, block_node)
            declarations.append(procedure_declaration)
            self.eat(SEMI)

        return declarations

    # variable_declaration : ID (COMMA ID)* COLON type_spec
    def variable_declaration(self):
        # ID
        variable_nodes = [Variable(self.current_token)]
        self.eat(ID)
        # (COMMA ID)*
        while (self.current_token.type_ == COMMA):
            self.eat(COMMA)
            variable_nodes.append(Variable(self.current_token))
            self.eat(ID)
        self.eat(COLON)

        type_node = self.type_spec()
        
        variable_declarations = [
            VariableDeclaration(variable_node, type_node)
            for variable_node in variable_nodes
        ]
        return variable_declarations

    # type_spec : INTEGER | REAL
    def type_spec(self):
        token = self.current_token
        if (self.current_token.type_ == INTEGER):
            self.eat(INTEGER)
        else:
            self.eat(REAL)

        node = Type(token)
        return node

    # compound_statement : BEGIN statement_list END
    def compound_statement(self):
        self.eat(BEGIN)
        nodes = self.statement_list()
        self.eat(END)

        root = Compound()
        for node in nodes:
            root.children.append(node)

        return root

    # statement_list : statement | statement SEMI statement_list
    def statement_list(self):
        node = self.statement()
        
        results = [node]
        while (self.current_token.type_ == SEMI):
            self.eat(SEMI)
            results.append(self.statement())

        # if (self.current_token.type_ == ID):
        #     self.error()

        return results

    # statement : compound_statement | assignment_statement | empty
    def statement(self):
        # is compound?
        if (self.current_token.type_ == BEGIN):
            node = self.compound_statement()
        # is assignment?
        elif (self.current_token.type_ == ID):
            node = self.assignment_statement()
        # otherwise
        else:
            node = self.empty()

        return node

    # assignment_statement : variable ASSIGN expr
    def assignment_statement(self):
        left = self.variable()
        token = self.current_token
        self.eat(ASSIGN)
        right = self.expr()

        node = Assign(left, token, right)
        return node

    # variable : ID
    def variable(self):
        node = Variable(self.current_token)
        self.eat(ID)

        return node

    # empty : 
    def empty(self):

        return Empty()

    # factor : PLUS factor | MINUS factor | INTEGER_CONST | REAL_CONST | LPAR expr RPAR | variable
    def factor(self):
        token = self.current_token
        # PLUS
        if (token.type_ == PLUS):
            self.eat(PLUS)
            node = UnaryOperator(token, self.factor())
            return node
        # MINUS
        elif (token.type_ == MINUS):
            self.eat(MINUS)
            node = UnaryOperator(token, self.factor())
            return node
        # INTEGER_CONST
        elif (token.type_ == INTEGER_CONST):
            self.eat(INTEGER_CONST)
            return Number(token)
        # REAL_CONST 
        elif (token.type_ == REAL_CONST):
            self.eat(REAL_CONST)
            return Number(token)
        # LPAR expr LPAR
        elif (token.type_ == LPAR):
            self.eat(LPAR)
            node = self.expr()
            self.eat(RPAR)
            return node
        # otherwise
        else:
            node = self.variable()
            return node

    # term : factor ((MULTIPLY | INTEGER_DIV | FLOAT_DIV) factor)*
    def term(self):
        node = self.factor()
        while (self.current_token.type_ in (MULTIPLY, INTEGER_DIV, FLOAT_DIV)):
            token = self.current_token
            # MULTIPLY
            if (token.type_ == MULTIPLY):
                self.eat(MULTIPLY)
            # INTEGER_DIV
            elif (token.type_ == INTEGER_DIV):
                self.eat(INTEGER_DIV)
            # FLOAT_DIV
            elif (token.type_ == FLOAT_DIV):
                self.eat(FLOAT_DIV)

            node = BinaryOperator(left=node, operator=token, right=self.factor())

        return node

    # expr : term ((PLUS | MINUS) term)*
    def expr(self):
        node = self.term()
        while (self.current_token.type_ in (PLUS, MINUS)):
            token = self.current_token
            # PLUS
            if (token.type_ == PLUS):
                self.eat(PLUS)
            # MINUS
            elif (token.type_ == MINUS):
                self.eat(MINUS)

            node = BinaryOperator(left=node, operator=token, right=self.term())

        return node

    def parse(self):
        '''
        program                 : PROGRAM variable SEMI block DOT
        
        block                   : declarations compound_statement
        
        declarations            : VAR (variable_declaration SEMI)+
                                | (PROCEDURE ID SEMI block SEMI)*
                                | empty
        
        variable_declaration    : ID (COMMA ID)* COLON type_spec
        
        type_spec               : INTEGER
                                | REAL
        
        compound_statement      : BEGIN statement_list END
        
        statement_list          : statement
                                | statement SEMI statement_list
        
        statement               : compound_statement
                                | assignment_statement
                                | empty
        
        assignment_statement    : variable ASSIGN expr
        
        empty                   : 
        
        expr                    : term ((PLUS | MINUS) term)*
        
        term                    : factor ((MULTIPLY | INTEGER_DIV | FLOAT_DIV) factor)*
        
        factor                  : PLUS factor
                                | MINUS factor
                                | INTEGER_CONST
                                | REAL_CONST
                                | LPAR expr RPAR
                                | variable
        
        variable                : ID
        '''
        node = self.program()
        if (self.current_token.type_ != EOF):
            self.error()

        return node

# ---------------------------------------------------
# semantic analysis (static)
# ---------------------------------------------------
class NodeVisitor(object):
    def visit(self, node):
        method_name = 'visit_' + type(node).__name__
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node):
        raise Exception('No visit_{} method'.format(type(node).__name__))

class SymbolTable(object):
    def __init__(self):
        self.symbols_ = {}
        # init builtins
        self.define(BuiltinTypeSymbol('INTEGER'))
        self.define(BuiltinTypeSymbol('REAL'))

    def __repr__(self):
        return 'Symbols: {symbols}'.format(
            symbols = [value for value in self.symbols_.values()])

    def define(self, symbol):
        # print('Define symbol {}'.format(symbol))
        self.symbols_[symbol.name] = symbol

    def lookup(self, name):
        # print('Lookup symbol {}'.format(name))
        symbol = self.symbols_.get(name)
        return symbol

class SymbolTableBuilder(NodeVisitor):
    def __init__(self):
        self.symbol_table = SymbolTable()

    def visit_Program(self, node):
        self.visit(node.block)

    def visit_Block(self, node):
        for declaration in node.declarations:
            self.visit(declaration)

        self.visit(node.compound_statement)

    def visit_ProcedureDeclaration(self, node):
        pass

    def visit_VariableDeclaration(self, node):
        # lookup builtin types
        type_name = node.type_node.value
        type_symbol = self.symbol_table.lookup(type_name)
        # define symbol in symbol table
        variable_name = node.variable_node.value
        variable_symbol = VarSymbol(variable_name, type_symbol)
        self.symbol_table.define(variable_symbol)

    def visit_Compound(self, node):
        for child in node.children:
            self.visit(child)

    def visit_Empty(self, node):
        pass

    def visit_Assign(self, node):
        variable_name = node.left.value
        variable_symbol = self.symbol_table.lookup(variable_name)
        if variable_symbol is None:
            raise NameError(repr(variable_name))

        self.visit(node.right)

    def visit_Variable(self, node):
        variable_name = node.value
        variable_symbol = self.symbol_table.lookup(variable_name)

        if (variable_symbol is None):
            raise NameError(repr(variable_name))

    def visit_BinaryOperator(self, node):
        self.visit(node.left)
        self.visit(node.right)

    def visit_UnaryOperator(self, node):
        self.visit(node.expr)

    def visit_Number(self, node):
        pass

# ---------------------------------------------------
# interpreter (program evaluation)
# ---------------------------------------------------
class Interpreter(NodeVisitor):
    def __init__(self, parser):
        self.parser = parser
        self.GLOBAL_SCOPE = {}

    def visit_Program(self, node):
        self.visit(node.block)

    def visit_Block(self, node):
        for declaration in node.declarations:
            self.visit(declaration)

        self.visit(node.compound_statement)

    def visit_ProcedureDeclaration(self, node):
        pass

    def visit_VariableDeclaration(self, node):
        pass

    def visit_Type(self, node):
        pass

    def visit_Compound(self, node):
        for child in node.children:
            self.visit(child)

    def visit_Empty(self, node):
        pass

    def visit_Assign(self, node):
        variable_name = node.left.value
        # add variable name and assigned value to global scope
        self.GLOBAL_SCOPE[variable_name] = self.visit(node.right)

    def visit_Variable(self, node):
        variable_name = node.value
        variable_value = self.GLOBAL_SCOPE.get(variable_name)

        if (variable_value is None):
            raise NameError(repr(variable_name))
        else:
            return variable_value

    def visit_BinaryOperator(self, node):
        if (node.operator.type_ == PLUS):
            return self.visit(node.left) + self.visit(node.right)
        elif (node.operator.type_ == MINUS):
            return self.visit(node.left) - self.visit(node.right)
        elif (node.operator.type_ == MULTIPLY):
            return self.visit(node.left) * self.visit(node.right)
        elif (node.operator.type_ == INTEGER_DIV):
            return self.visit(node.left) // self.visit(node.right)
        elif (node.operator.type_ == FLOAT_DIV):
            return float(self.visit(node.left) / self.visit(node.right))

    def visit_UnaryOperator(self, node):
        if (node.operator.type_ == PLUS):
            return +self.visit(node.expr)
        elif (node.operator.type_ == MINUS):
            return -self.visit(node.expr)

    def visit_Number(self, node):
        
        return node.value

    def interpret(self):
        tree = self.parser.parse()
        # symbol table
        symbol_table_builder = SymbolTableBuilder()
        symbol_table_builder.visit(tree)
        # print(symbol_table_builder.symbol_table)
        pt = PrettyTable(['Symbol', 'Type'])
        for k, v in sorted(symbol_table_builder.symbol_table.symbols_.items()):
            pt.add_row([k, v])
        print(pt)
        if (tree is None):
            return ''

        return self.visit(tree)

def main():
    with open('test_proc.pas', 'r') as file:
        text = file.read()
        # print(text)

        # lexer
        lexer = Lexer(text)
        # parser
        parser = Parser(lexer)
        # semantic

        # evaluate
        # interpreter = None
        interpreter = Interpreter(parser)
        interpreter.interpret()

        # print global scope
        if interpreter:
            pt = PrettyTable(['ID', 'Value (run-time)'])
            for k, v in sorted(interpreter.GLOBAL_SCOPE.items()):
                # print('{} = {}'.format(k, v))
                pt.add_row([k, v])
            print(pt)

if (__name__ == '__main__'):
    main()
