// test: integer arrays should behave like expected

const arraygen = require('../include.array.js');

const MAX_NUM = 1e5;
const MIN_NUM = -1e5;

function getRandomInt() {
    return Math.floor(Math.random() * (MAX_NUM - MIN_NUM)) + MIN_NUM;
}

const SIZE = [ 10, 10, 10, 10 ];

arraygen.generateOutput(SIZE, getRandomInt, '%');
