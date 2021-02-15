
const MAX_NUM = 1e5;
const MIN_NUM = -1e5;

function getRandomInt() {
    return Math.floor(Math.random() * (MAX_NUM - MIN_NUM)) + MIN_NUM;
}

function generateCondition() {
    const a = getRandomInt();
    const b = getRandomInt();
    const rand = Math.random();
    if (rand < 0.16) {
        return [`${a} = ${b}`, a == b];
    } else if (rand < 0.32) {
        return [`${a} != ${b}`, a != b];
    } else if (rand < 0.48) {
        return [`${a} < ${b}`, a < b];
    } else if (rand < 0.64) {
        return [`${a} <= ${b}`, a <= b];
    } else if (rand < 0.80) {
        return [`${a} > ${b}`, a > b];
    } else {
        return [`${a} >= ${b}`, a >= b];
    }
}

function generateOutput(ldetph, rdetph, maxdetph) {
    if (ldetph > 0 && rdetph > 0 && maxdetph > 0) {
        const [out_a, exp_a] = generateOutput(ldetph - 1, rdetph, maxdetph - 1);
        const [out_b, exp_b] = generateOutput(ldetph, rdetph - 1, maxdetph - 1);
        const [out_cond, exp_cond] = generateCondition();
        return [`(If ${out_cond} Then ${out_a} Else ${out_b})`, exp_cond ? exp_a : exp_b];
    } else {
        const val = getRandomInt();
        return [val.toString(), val];
    }
}

module.exports = {
    generateOutput: generateOutput
};
