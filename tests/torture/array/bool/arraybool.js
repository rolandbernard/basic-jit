// test: boolean arrays should behave like expected

const arraygen = require('../include.array.js');

function getRandomBool() {
    return Math.random() < 0.5 ? 'True' : 'False';
}

const SIZE = [ 10, 10, 10, 10 ];

arraygen.generateOutput(SIZE, getRandomBool, '?');
