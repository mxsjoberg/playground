/*
    AST transformer
*/

class Transformer {

    /*
        Transform `def`-expression into variable declaration with lambda expression
    */
    transformDefToVarLambda(exp) {
        const [_tag, name, params, body] = exp;
        return ["var", name, ["lambda", params, body]];
    }

    /*
        Transform `switch`-expression to nested `if`-expressions
    */
    transformSwitchToIf(exp) {
        const [_tag, ...cases] = exp;

        const ifExp = ["if", null, null, null];
        let current = ifExp;
        
        for (let i = 0; i < cases.length - 1; i++) {
            const [currentCond, currentBlock] = cases[i];

            current[1] = currentCond;
            current[2] = currentBlock;

            const next = cases[i + 1];
            const [nextCond, nextBlock] = next;

            current[3] = nextCond === 'else' ? nextBlock : ['if'];

            current = current[3];
        }

        return ifExp;
    }

    /*
        Transform `for`-loop to `while`-loop
    */
    transformForToWhile(exp) {
        const [_tag, init, condition, modifier, body] = exp;
        return ["begin", init, ["while", condition, ["begin", body, modifier]]];
    }

    /*
        Transform `++`-expression to `set`-expression
    */
    transformIncToSet(exp) {
        const [_tag, name] = exp;
        return ["set", name, ["+", name, 1]];
    }

    /*
        Transform `--`-expression to `set`-expression
    */
    transformDecToSet(exp) {
        const [_tag, name] = exp;
        return ["set", name, ["-", name, 1]];
    }
}

module.exports = Transformer;
