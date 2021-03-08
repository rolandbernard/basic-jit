// test: integers should behave like expected

const intgen = require('./include.intgen.js');

const NUM_TEST = 5000;

console.log('');

for (let i = 0; i < NUM_TEST; i++) {
    const [calc, expected] = intgen.generateOutput(1, 1, Infinity);
    console.log(`Assert ${calc} = ${expected.toString()}\n`);
}