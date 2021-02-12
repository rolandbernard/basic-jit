
const boolgen = require('./include.boolgen.js');

const [calc, expected] = boolgen.generateOutput(1, 1000, Infinity);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
