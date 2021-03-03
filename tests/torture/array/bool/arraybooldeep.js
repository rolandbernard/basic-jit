
const arraygen = require('../include.array.js');

function getRandomBool() {
    return Math.random() < 0.5 ? 'True' : 'False';
}

const SIZE = [ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 ];

arraygen.generateOutput(SIZE, getRandomBool, '?');
