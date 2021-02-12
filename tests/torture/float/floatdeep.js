
const floatgen = require('./include.floatgen.js');

const EPSILON = 1e-7;

const [calc, expected] = floatgen.generateOutput(Infinity, Infinity, 12);
console.log(`\nAssert Abs(${calc} - ${expected.toString()}) <= ${Math.abs(expected * EPSILON)}\n`);
