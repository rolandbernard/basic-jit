// test: integers should behave like expected

const intgen = require('./include.intgen.js');

const [calc, expected] = intgen.generateOutput(4, 40, Infinity);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
