const Eva = require("../Eva");
const Environment = require("../Environment");

const tests = [
    require("./self-eval-test.js"),
    require("./arithmetics-test.js"),
    require("./variables-test.js"),
    require("./block-test.js"),
    require("./if-test.js"),
    require("./while-test.js"),
];

const eva = new Eva(new Environment({
    // pre-defined variables
    null: null,

    true: true,
    false: false,

    VERSION: '0.1',
}));

tests.forEach(test => test(eva));

console.log("all tests passed");