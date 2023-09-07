const assert = require("assert");

const Environment = require("./Environment");

/*

    Eva interpreter

    Exp ::= Number
          | String
          | [+ Exp, Exp]
          ...
          | [var name, Exp]
          | [set name, Exp]
          | name
          ...
          | [begin Exp...]
          ...
          | [if Exp Exp Exp]
          | [while Exp Exp]

*/

class Eva {

    // Create instance with global environment
    
    constructor(global = GlobalEnvironment) {
        this.global = global;
    }

    // Evaluate expression in given environment
    
    eval(exp, env = this.global) {
        
        // Self-evaluating expressions
        
        if (this._isNumber(exp)) {
            return exp;
        }
        if (this._isString(exp)) {
            return exp.slice(1, -1);
        }
        
        // Math operators
        
        // if (exp[0] === '+') {
        //     return this.eval(exp[1], env) + this.eval(exp[2], env);
        // }
        // if (exp[0] === '*') {
        //     return this.eval(exp[1], env) * this.eval(exp[2], env);
        // }

        // Comparison operators

        // if (exp[0] === '>') {
        //     return this.eval(exp[1], env) > this.eval(exp[2], env);
        // }

        // if (exp[0] === '>=') {
        //     return this.eval(exp[1], env) >= this.eval(exp[2], env);
        // }

        // if (exp[0] === '<') {
        //     return this.eval(exp[1], env) < this.eval(exp[2], env);
        // }

        // if (exp[0] === '<=') {
        //     return this.eval(exp[1], env) >= this.eval(exp[2], env);
        // }

        // if (exp[0] === '=') {
        //     return this.eval(exp[1], env) === this.eval(exp[2], env);
        // }

        // Block

        if (exp[0] === 'begin') {
            const blockEnv = new Environment({}, env);
            return this._evalBlock(exp, blockEnv);
        }
        
        // Variable declaration
        
        if (exp[0] === 'var') {
            const [_, name, value] = exp;
            return env.define(name, this.eval(value, env));
        }

        // Variable assignment

        if (exp[0] === 'set') {
            const [_, name, value] = exp;
            return env.assign(name, this.eval(value, env));
        }

        // Variable access

        if (this._isVariableName(exp)) {
            return env.lookup(exp);
        }

        // If-expression

        if (exp[0] === 'if') {
            const [_tag, condition, consequent, alternate] = exp;
            if (this.eval(condition, env)) {
                return this.eval(consequent, env);
            }
            return this.eval(alternate, env);
        }

        // While-expression

        if (exp[0] === 'while') {
            const [_tag, condition, body] = exp;
            let result;
            while (this.eval(condition, env)) {
                result = this.eval(body, env);
            }
            return result;
        }

        // function declaration: (def square (x) (* x x))
        /*
            syntactic sugar for: (var square (lambda (x) (* x x)))
        */

        if (exp[0] === 'def') {
            const [_tag, name, params, body] = exp;
            // including global env -> closure
            // const fn = { params, body, env };

            // return env.define(name, fn);

            // JIT-transpile to variable declaration
            const varExp = ["var", name, ["lambda", params, body]];
            return this.eval(varExp, env);
        }

        // Lambda function: (lambda (x) (* x x))

        if (exp[0] === 'lambda') {
            const [_tag, params, body] = exp;
            return {
                params,
                body,
                env,
            };
        }

        // function call
        /*
            (print "hello eva")
            (+ x 5)
            (> x y)
        */

        if (Array.isArray(exp)) {
            const fn = this.eval(exp[0], env);
            const args = exp.slice(1).map(arg => this.eval(arg, env));

            // native function
            if (typeof fn === 'function') {
                return fn(...args);
            }

            // user-defined function
            const activationRecord = {};
            fn.params.forEach((param, index) => {
                activationRecord[param] = args[index];
            });
            const activationEnvironment = new Environment(
                activationRecord,
                fn.env,
            );
            return this._evalBody(fn.body, activationEnvironment);
        }
        
        throw `TODO: ${JSON.stringify(exp)}`;
    }

    // Helpers

    _evalBody(body, env) {
        if (body[0] === "begin") {
            return this._evalBlock(body, env);
        }
        return this.eval(body, env);
    }

    _evalBlock(block, env) {
        let result;
        const [_tag, ...expressions] = block;

        expressions.forEach(exp => {
            result = this.eval(exp, env);
        });

        return result;
    }

    _isNumber(exp) {
        return typeof exp === "number";
    }

    _isString(exp) {
        return typeof exp === "string" && exp[0] === '"' && exp.slice(-1) === '"';
    }

    _isVariableName(exp) {
        return typeof exp === "string" && /^[+\-*/<>=a-zA-Z0-9_]+$/.test(exp);
    }
}

// Default Global Environment

const GlobalEnvironment = new Environment({
    VERSION: '0.1',
    
    null: null,
    true: true,
    false: false,

    // operators

    '+'(op1, op2) {
        return op1 + op2;
    },

    '*'(op1, op2) {
        return op1 * op2;
    },

    '-'(op1, op2=null) {
        if (op2 == null) {
            return -op1;
        }
        return op1 - op2;
    },

    '/'(op1, op2) {
        return op1 / op2;
    },

    // comparison

    '>'(op1, op2) {
        return op1 > op2;
    },

    '<'(op1, op2) {
        return op1 < op2;
    },

    '>='(op1, op2) {
        return op1 >= op2;
    },

    '<='(op1, op2) {
        return op1 <= op2;
    },

    '='(op1, op2) {
        return op1 === op2;
    },

    // Console output

    print(...args) {
        console.log(...args);
    },

});

module.exports = Eva;
