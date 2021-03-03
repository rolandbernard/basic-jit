
const arraygen = require('../include.array.js');

function getRandomString() {
    return `"${Math.random().toString(36).substr(2)}"`;
}

const SIZE = [ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 ];

arraygen.generateOutput(SIZE, getRandomString, '$');
