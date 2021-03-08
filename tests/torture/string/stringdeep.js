// test: strings should behave like expected

const stringgen = require('./include.stringgen.js');

const [calc, expected] = stringgen.generateOutput(Infinity, Infinity, 12);
console.log(`\nAssert ${calc} = "${expected.toString()}"\n`);
