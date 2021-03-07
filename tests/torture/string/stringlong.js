// test: strings should behave like expected

const stringgen = require('./include.stringgen.js');

const [calc, expected] = stringgen.generateOutput(1000, 1, Infinity);
console.log(`\nAssert ${calc} = "${expected.toString()}"\n`);
