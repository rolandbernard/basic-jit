
const ifgen = require('./include.ifgen.js');

const NUM_TEST = 5000;

console.log('');

for (let i = 0; i < NUM_TEST; i++) {
    const [calc, expected] = ifgen.generateOutput(1, 1, Infinity);
    console.log(`Assert ${calc} = ${expected.toString()}\n`);
}