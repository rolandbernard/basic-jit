
const ifgen = require('./include.ifgen.js');

const [calc, expected] = ifgen.generateOutput(1, 1000, Infinity);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
