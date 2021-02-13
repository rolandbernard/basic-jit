
const funcgen = require('./include.funcgen.js');

const EPSILON = 1e-7;

const [calc, expected] = funcgen.generateOutput(1000, 1, Infinity);
console.log(`\nDef Fn square(X) = X*X\nAssert Abs(${calc} - ${expected.toString()}) <= ${Math.abs(expected * EPSILON)}\n`);
