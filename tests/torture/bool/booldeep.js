// test: booleans should behave like expected

const boolgen = require('./include.boolgen.js');

const [calc, expected] = boolgen.generateOutput(Infinity, Infinity, 12);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
