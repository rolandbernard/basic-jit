
const MAX_NUM = 1e5;
const MIN_NUM = -1e5;

function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min)) + min;
}

function getRandomString() {
    return Math.random().toString(36).substr(2);
}

function generateOutput(ldetph, rdetph, maxdetph) {
    if (ldetph > 0 && rdetph > 0 && maxdetph > 0) {
        const [out_a, exp_a] = generateOutput(ldetph - 1, rdetph, maxdetph - 1);
        const [out_b, exp_b] = generateOutput(ldetph, rdetph - 1, maxdetph - 1);
        const int = getRandomInt(0, exp_a.length);
        for (;;) {
            let rand = Math.random();
            if (rand < 0.3) {
                return [`Right(${out_a}, ${int})`, exp_a.substr(exp_a.length - int)];
            } else if (rand < 0.6) {
                return [`Left(${out_a}, ${int})`, exp_a.substr(0, int)];
            } else {
                return [`(${out_a} + ${out_b})`, exp_a + exp_b];
            }
        }
    } else {
        let rand = Math.random();
        if (rand < 0.5) {
            const val = getRandomString();
            return [`"${val}"`, val];
        } else {
            const int = getRandomInt(MIN_NUM, MAX_NUM);
            return [`Str(${int})`, int.toString()];
        }
    }
}

module.exports = {
    generateOutput: generateOutput
};
