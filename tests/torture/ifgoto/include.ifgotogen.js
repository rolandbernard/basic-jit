
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

let if_count = 0;
function generateOutput(ldetph, rdetph, maxdetph, incorrect) {
    const if_number = if_count;
    if_count++;
    if (ldetph > 0 && rdetph > 0 && maxdetph > 0) {
        const [out_cond, exp_cond] = generateCondition();
        const out_a = generateOutput(ldetph - 1, rdetph, maxdetph - 1, incorrect || !exp_cond);
        const out_b = generateOutput(ldetph, rdetph - 1, maxdetph - 1, incorrect || exp_cond);
        return `If ${out_cond} Then Goto if_label_${if_number} Else Goto else_label_${if_number}
if_label_${if_number}:
${out_a}
else_label_${if_number}:
${out_b}`;
    } else {
        if (incorrect) {
            return `End ${if_number}`;
        } else {
            return `End`;
        }
    }
}

module.exports = {
    generateOutput: generateOutput
};
