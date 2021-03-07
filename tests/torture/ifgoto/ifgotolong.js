// test: ifgoto should behave like expected

const ifgen = require('./include.ifgotogen.js');

const out = ifgen.generateOutput(1, 1000, Infinity);
console.log(`\n${out}\n`);
