// Or.tst

// load 'Or.hdl'
load Or.hdl;

// define output-file 
output-file Or.out,
output-list a b out;

// tests
set a 0,
set b 0, 
eval,
output;

set a 0,
set b 1,
eval,
output;

set a 1,
set b 0,
eval,
output;

set a 1,
set b 1,
eval,
output;