const assert = require("assert");
const {test} = require("./test-util");

module.exports = eva => {

    test(eva, `
    (begin
        (import Math)

        ((prop Math abs) (- 10))
    )
    `, 10);

    test(eva, `
    (begin
        (var abs (prop Math abs))

        (abs (- 10))
    )
    `, 10);

    test(eva, `
    (begin
        (prop Math MAX_VALUE)
    )
    `, 1000);

};