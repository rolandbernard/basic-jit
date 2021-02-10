
const EXPECTED_DEPTH = 2;
const MAX_NUM = 1e5;
const MIN_NUM = -1e5;

function getRandomInt() {
    return Math.floor(Math.random() * (MAX_NUM - MIN_NUM)) + MIN_NUM;
}

function between(a, low, high) {
    return a <= high && a >= low;
}

function generateOutput(detph) {
    if (detph > 0) {
        const [out_a, exp_a] = generateOutput(detph - 1);
        const [out_b, exp_b] = generateOutput(detph - 1);
        for (;;) {
            let rand = Math.random();
            if (rand < 0.2 && between(exp_a + exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} + ${out_b})`, exp_a + exp_b];
            } else if (rand < 0.4 && between(exp_a - exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} - ${out_b})`, exp_a - exp_b];
            } else if (rand < 0.6 && between(exp_a * exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} * ${out_b})`, exp_a * exp_b];
            } else if (rand < 0.8 && between(Math.trunc(exp_a / exp_b), MIN_NUM, MAX_NUM)) {
                return [`(${out_a} / ${out_b})`, Math.trunc(exp_a / exp_b)];
            } else if (between(exp_a % exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} mod ${out_b})`, exp_a % exp_b];
            }
        }
    } else {
        const val = getRandomInt();
        return [val.toString(), val];
    }
}

const [calc, expected] = generateOutput(12);
console.log('\nAssert ' + calc + ' = ' + expected.toString() + '\n');
