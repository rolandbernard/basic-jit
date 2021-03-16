
const MAX_NUM = 1e10;
const MIN_NUM = -1e10;

function getRandomFloat() {
    return Math.random() * (MAX_NUM - MIN_NUM) + MIN_NUM;
}

function between(a, low, high) {
    return a <= high && a >= low;
}

function generateOutput(ldetph, rdetph, maxdetph) {
    if (ldetph > 0 && rdetph > 0 && maxdetph > 0) {
        const [out_a, exp_a] = generateOutput(ldetph - 1, rdetph, maxdetph - 1);
        const [out_b, exp_b] = generateOutput(ldetph, rdetph - 1, maxdetph - 1);
        for (;;) {
            let rand = Math.random();
            if (rand < 0.25 && between(exp_a + exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} + ${out_b})`, exp_a + exp_b];
            } else if (rand < 0.5 && between(exp_a - exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} - ${out_b})`, exp_a - exp_b];
            } else if (rand < 0.75 && between(exp_a * exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} * ${out_b})`, exp_a * exp_b];
            } else if (between(exp_a / exp_b, MIN_NUM, MAX_NUM)) {
                return [`(${out_a} / ${out_b})`, exp_a / exp_b];
            }
        }
    } else {
        const val = getRandomFloat();
        let string = val.toString();
        if (!string.includes('e') && !string.includes('.')) {
            string += '.0';
        }
        return [string, val];
    }
}

module.exports = {
    generateOutput: generateOutput
};