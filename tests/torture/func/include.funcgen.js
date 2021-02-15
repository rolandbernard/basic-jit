

const MAX_NUM = 1e5;
const MIN_NUM = -1e5;

function getRandomFloat() {
    return Math.random() * (MAX_NUM - MIN_NUM) + MIN_NUM;
}

function between(a, low, high) {
    return a <= high && a >= low;
}

function generateOutput(ldetph, rdetph, maxdetph) {
    if (ldetph > 0 && rdetph > 0 && maxdetph > 0) {
        const [out_a, exp_a] = generateOutput(ldetph - 1, rdetph, maxdetph - 1);
        for (;;) {
            let rand = Math.random();
            if (rand < 0.06 && between(Math.abs(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Abs(${out_a})`, Math.abs(exp_a)];
            } else if (rand < 0.12 && between(Math.acos(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Acs(${out_a})`, Math.acos(exp_a)];
            } else if (rand < 0.18 && between(Math.asin(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Asn(${out_a})`, Math.asin(exp_a)];
            } else if (rand < 0.24 && between(Math.atan(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Atn(${out_a})`, Math.atan(exp_a)];
            } else if (rand < 0.30 && between(Math.cos(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Cos(${out_a})`, Math.cos(exp_a)];
            } else if (rand < 0.36 && between(Math.sin(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Sin(${out_a})`, Math.sin(exp_a)];
            } else if (rand < 0.42 && between(Math.tan(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Tan(${out_a})`, Math.tan(exp_a)];
            } else if (rand < 0.48 && between(Math.exp(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Exp(${out_a})`, Math.exp(exp_a)];
            } else if (rand < 0.54 && between(exp_a % 1.0, MIN_NUM, MAX_NUM)) {
                return [`Frac(${out_a})`, exp_a % 1.0];
            } else if (rand < 0.60 && between(Math.trunc(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Int(${out_a})`, Math.trunc(exp_a)];
            } else if (rand < 0.66 && between(Math.log(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Ln(${out_a})`, Math.log(exp_a)];
            } else if (rand < 0.72 && between(Math.log10(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Log(${out_a})`, Math.log10(exp_a)];
            } else if (rand < 0.78 && between(Math.round(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Rnd(${out_a})`, Math.round(exp_a)];
            } else if (rand < 0.84 && between(Math.sign(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Sgn(${out_a})`, Math.sign(exp_a)];
            } else if (rand < 0.90 && between(Math.sqrt(exp_a), MIN_NUM, MAX_NUM)) {
                return [`Sqr(${out_a})`, Math.sqrt(exp_a)];
            } else if (between(exp_a*exp_a, MIN_NUM, MAX_NUM)) {
                return [`Fn square(${out_a})`, exp_a*exp_a];
            }
        }
    } else {
        const val = getRandomFloat();
        return [val.toString(), val];
    }
}

module.exports = {
    generateOutput: generateOutput
};
