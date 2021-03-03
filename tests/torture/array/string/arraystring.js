
const arraygen = require('../include.array.js');

function getRandomString() {
    return `"${Math.random().toString(36).substr(2)}"`;
}

const SIZE = [ 10, 10, 10, 10 ];

arraygen.generateOutput(SIZE, getRandomString, '$');
