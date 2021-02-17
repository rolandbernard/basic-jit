
const ifgen = require('./include.ifgotogen.js');

const out = ifgen.generateOutput(3, 30, Infinity);
console.log(`\n${out}\n`);
