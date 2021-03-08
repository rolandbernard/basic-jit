// test: floats should behave like expected

const floatgen = require('./include.floatgen.js');

const EPSILON = 1e-7;

const [calc, expected] = floatgen.generateOutput(4, 40, Infinity);
console.log(`\nAssert Abs(${calc} - ${expected.toString()}) <= ${Math.abs(expected * EPSILON)}\n`);
