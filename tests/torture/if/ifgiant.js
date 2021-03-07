// test: if should behave like expected

const ifgen = require('./include.ifgen.js');

const [calc, expected] = ifgen.generateOutput(3, 40, Infinity);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
