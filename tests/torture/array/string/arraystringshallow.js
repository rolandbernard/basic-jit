// test: string arrays should behave like expected

const arraygen = require('../include.array.js');

function getRandomString() {
    return `"${Math.random().toString(36).substr(2)}"`;
}

const SIZE = [ 12345 ];

arraygen.generateOutput(SIZE, getRandomString, '$');
