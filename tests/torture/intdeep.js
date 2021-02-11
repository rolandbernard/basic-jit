
const intgen = require('./include.intgen.js');

const [calc, expected] = intgen.generateOutput(Infinity, Infinity, 12);
console.log(`\nAssert ${calc} = ${expected.toString()}\n`);
