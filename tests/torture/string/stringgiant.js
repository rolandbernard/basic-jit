// test: strings should behave like expected

const stringgen = require('./include.stringgen.js');

const [calc, expected] = stringgen.generateOutput(40, 4, Infinity);
console.log(`\nAssert ${calc} = "${expected.toString()}"\n`);
