// And.tst

// load 'And.hdl'
load And.hdl;

// define output-file 
output-file And.out,
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