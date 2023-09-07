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
    
    constructor(global = new Environment()) {
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
        
        if (exp[0] === '+') {
            return this.eval(exp[1], env) + this.eval(exp[2], env);
        }
        if (exp[0] === '*') {
            return this.eval(exp[1], env) * this.eval(exp[2], env);
        }

        // Comparison operators

        if (exp[0] === '>') {
            return this.eval(exp[1], env) > this.eval(exp[2], env);
        }

        if (exp[0] === '>=') {
            return this.eval(exp[1], env) >= this.eval(exp[2], env);
        }

        if (exp[0] === '<') {
            return this.eval(exp[1], env) < this.eval(exp[2], env);
        }

        if (exp[0] === '<=') {
            return this.eval(exp[1], env) >= this.eval(exp[2], env);
        }

        if (exp[0] === '=') {
            return this.eval(exp[1], env) === this.eval(exp[2], env);
        }

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
        
        throw `TODO: ${JSON.stringify(exp)}`;
    }

    // Helpers

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
        return typeof exp === "string" && /^[a-zA-Z][a-zA-Z0-9_]*$/.test(exp);
    }
}

module.exports = Eva;
