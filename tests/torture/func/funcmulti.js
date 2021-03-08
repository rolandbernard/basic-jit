// test: functions should behave like expected

const funcgen = require('./include.funcgen.js');

const EPSILON = 1e-5;
const NUM_TEST = 5000;

console.log('\nDef Fn square(X) = X*X\n');

for (let i = 0; i < NUM_TEST; i++) {
    const [calc, expected] = funcgen.generateOutput(1, 1, Infinity);
    console.log(`Assert Abs(${calc} - ${expected.toString()}) <= ${Math.max(Math.abs(expected * EPSILON), EPSILON)}\n`);
}
