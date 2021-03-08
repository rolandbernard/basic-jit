// test: booleans should behave like expected

const boolgen = require('./include.boolgen.js');

const [calc, expected] = boolgen.generateOutput(4, 40, Infinity);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
