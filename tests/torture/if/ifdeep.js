
const ifgen = require('./include.ifgen.js');

const [calc, expected] = ifgen.generateOutput(Infinity, Infinity, 12);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
