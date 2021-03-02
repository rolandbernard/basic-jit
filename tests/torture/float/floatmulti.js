
const floatgen = require('./include.floatgen.js');

const EPSILON = 1e-7;

const NUM_TEST = 5000;

console.log('');

for (let i = 0; i < NUM_TEST; i++) {
    const [calc, expected] = floatgen.generateOutput(1, 1, Infinity);
    console.log(`Assert Abs(${calc} - ${expected.toString()}) <= ${Math.abs(expected * EPSILON)}\n`);
}
