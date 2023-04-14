// Not.tst

// load 'Not.hdl'
load Not.hdl;

// define output-file 
output-file Not.out,
output-list in out;

// tests
set in 0,
eval,
output;

set in 1,
eval,
output;