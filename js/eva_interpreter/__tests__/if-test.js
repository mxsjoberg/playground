const assert = require("assert");

// (if <condition> <consequent> <alternate>)

module.exports = eva => {
    assert.strictEqual(eva.eval(
        ['begin',
            ['var', 'x', 10],
            ['var', 'y', 0],
            ['if', ['>', 'x', 10],  // condition
                ['set', 'y', 20],   // consequent
                ['set', 'y', 30],   // alternate
            ],
            'y'
        ]),
    30);
}