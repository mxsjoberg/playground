const Environment = require("./Environment");
const Transformer = require("./Transformer");
const evaParser = require("./parser/evaParser");
// file system module
const fs = require("fs");

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

    chmod +x bin/eva
    
    ./bin/eva -e '(begin (var x 10) (print 10))'
    ./bin/eva -e '(begin (var x 10) (print (* x 15)))'
    ./bin/eva -e '(begin ((lambda (x) (* x x)) 2))'
    ./bin/eva -e '(begin (print ((lambda (x) (* x x)) 2)))'

    ./bin/eva -f ./test.eva
*/

class Eva {

    /*
        Create instance with global environment `global`
    */
    constructor(global = GlobalEnvironment) {
        this.global = global;
        this._transformer = new Transformer();
    }

    /*
        Evaluate global code wrapping into a block
    */
    evalGlobal(exp) {
        return this._evalBody(exp, this.global);
    }

    /*
        Evaluate expression `expr` in given environment `env`
    */
    eval(exp, env = this.global) {
        
        /*
            Self-evaluating expressions
        */
        if (this._isNumber(exp)) {
            return exp;
        }
        if (this._isString(exp)) {
            return exp.slice(1, -1);
        }
        
        /*
            Math operators (moved to environment)
        */
        // if (exp[0] === '+') {
        //     return this.eval(exp[1], env) + this.eval(exp[2], env);
        // }
        // if (exp[0] === '*') {
        //     return this.eval(exp[1], env) * this.eval(exp[2], env);
        // }

        /*
            Comparison operators (moved to environment)
        */
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

        /*
            Block
        */
        if (exp[0] === 'begin') {
            const blockEnv = new Environment({}, env);
            return this._evalBlock(exp, blockEnv);
        }
        
        /*
            Variable declaration:

                (var x 10)
        */
        if (exp[0] === 'var') {
            const [_, name, value] = exp;
            return env.define(name, this.eval(value, env));
        }

        /*
            Variable assignment:

                (set x 10)
        */
        if (exp[0] === 'set') {
            const [_, name, value] = exp;

            // assignment to property
            if (name[0] === 'prop') {
                const [_tag, instance, propName] = name;
                const instanceEnv = this.eval(instance, env);

                return instanceEnv.define(
                    propName,
                    this.eval(value, env),
                );
            }

            return env.assign(name, this.eval(value, env));
        }

        /*
            Variable access
        */

        if (this._isVariableName(exp)) {
            return env.lookup(exp);
        }

        /*
            If-expression
        */
        if (exp[0] === 'if') {
            const [_tag, condition, consequent, alternate] = exp;
            if (this.eval(condition, env)) {
                return this.eval(consequent, env);
            }
            return this.eval(alternate, env);
        }

        /*
            While-expression
        */
        if (exp[0] === 'while') {
            const [_tag, condition, body] = exp;
            let result;
            while (this.eval(condition, env)) {
                result = this.eval(body, env);
            }
            return result;
        }

        /*
            Function declaration:

                (def square (x) (* x x))

            , syntactic sugar for:

                (var square (lambda (x) (* x x)))
        */
        if (exp[0] === 'def') {
            const [_tag, name, params, body] = exp;
            // including global env -> closure
            // const fn = { params, body, env };

            // return env.define(name, fn);

            // JIT-transpile to variable declaration
            // const varExp = ["var", name, ["lambda", params, body]];
            const varExp = this._transformer.transformDefToVarLambda(exp);
            return this.eval(varExp, env);
        }

        /*
            Switch-expression:

                (switch (cond1, block1) ...)

            , syntactic sugar for nested if-expressions
        */
        if (exp[0] === 'switch') {
            const ifExp = this._transformer.transformSwitchToIf(exp);
            return this.eval(ifExp, env);
        }

        /*
            For-loop:

                (for init condition modifier body)

            , syntactic sugar for:

                (begin init (while condition (begin body modifier)))
        */
        if (exp[0] === 'for') {
            const whileExp = this._transformer.transformForToWhile(exp);
            return this.eval(whileExp, env);
        }

        /* 
            Increment:

                (++ x)

            , syntactic sugar for:

                (set x (+ x 1))
        */
        if (exp[0] === '++') {
            const setExp = this._transformer.transformIncToSet(exp);
            return this.eval(setExp, env);
        }

        /*
            Decrement:

                (-- x)

            , syntactic sugar for:

                (set x (- x 1))
        */
        if (exp[0] === '--') {
            const setExp = this._transformer.transformDecToSet(exp);
            return this.eval(setExp, env);
        }

        /*
            Lambda function:

                (lambda (x) (* x x))
        */
        if (exp[0] === 'lambda') {
            const [_tag, params, body] = exp;
            return {
                params,
                body,
                env,
            };
        }

        /*
            Class declaration:

                (class Point null <body>)
        */
        if (exp[0] === 'class') {
            const [_tag, name, parent, body] = exp;
            // create environment for class (class is environment)
            const parentEnv = this.eval(parent, env) || env;
            const classEnv = new Environment({}, parentEnv);
            // evaluate class body in class environment
            this._evalBody(body, classEnv);
            // define class in current environment
            return env.define(name, classEnv);
        }

        /*
            Super expressions:

                (super <ClassName>)
        */
        if (exp[0] === 'super') {
            const [_tag, className] = exp;
            return this.eval(className, env).parent;
        }

        /*
            Class initialization:

                (new Point 1 2)
        */
        if (exp[0] === 'new') {
            const classEnv = this.eval(exp[1], env);
            // create instance of class (instance of class is another environment)
            const instanceEnv = new Environment({}, classEnv);

            const args = exp.slice(2).map(arg => this.eval(arg, env));

            this._callUserDefinedFunction(classEnv.lookup('constructor'), [instanceEnv, ...args]);

            return instanceEnv;
        }

        /*
            Property access:

                (prop p calc)
        */
        if (exp[0] === 'prop') {
            const [_tag, instance, name] = exp;

            const instanceEnv = this.eval(instance, env);

            return instanceEnv.lookup(name);
        }

        /*
            Module declaration:

                (module Math <body>)
        */
        if (exp[0] === 'module') {
            const [_tag, name, body] = exp;
            // module is named first-class environment
            const moduleEnv = new Environment({}, env);

            this._evalBody(body, moduleEnv);

            return env.define(name, moduleEnv);
        }

        /*
            Module import:

                (import Math)
        */
        if (exp[0] === 'import') {
            const [_tag, name] = exp;

            const moduleSource = fs.readFileSync(
                `${__dirname}/modules/${name}.eva`,
                'utf-8',
            );

            const body = evaParser.parse(`(begin ${moduleSource})`);

            const moduleExp = ['module', name, body];

            return this.eval(moduleExp, this.global);
        }

        /*
            Function call:
                
                (print "hello eva")
                (+ x 5)
                (> x y)
        */
        if (Array.isArray(exp)) {
            const fn = this.eval(exp[0], env);
            const args = exp.slice(1).map(arg => this.eval(arg, env));
            // native function (built-in)
            if (typeof fn === 'function') {
                return fn(...args);
            }
            // user-defined function
            return this._callUserDefinedFunction(fn, args);
        }
        /*
            Not implemented
        */
        throw `TODO: ${JSON.stringify(exp)}`;
    }

    /*
        Helpers
    */
    _callUserDefinedFunction(fn, args) {
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

/*
    Default Global Environment
*/
const GlobalEnvironment = new Environment({
    VERSION: '0.1',
    // system
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
