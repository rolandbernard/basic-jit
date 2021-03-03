
const arraygen = require('../include.array.js');

const MAX_NUM = 1e300;
const MIN_NUM = -1e300;

function getRandomFloat() {
    return Math.random() * (MAX_NUM - MIN_NUM) + MIN_NUM;
}

const SIZE = [ 12345 ];

arraygen.generateOutput(SIZE, getRandomFloat, '.');
