
function generateOutput(size, generator, type) {
    console.log(`\nDim A${type}(${size.join(', ')})\n`);

    let expected = [];

    function generateWrites(loc, d) {
        if (d < size.length) {
            for (let i = 0; i < size[d]; i++) {
                generateWrites(loc.concat(i), d + 1);
            }
        } else {
            const value = generator();
            console.log(`Let A(${loc.join(', ')}) = ${value}`);
            expected.push(value);
        }
    }

    generateWrites([], 0);

    console.log();

    function generateAsserts(loc, d) {
        if (d < size.length) {
            for (let i = 0; i < size[d]; i++) {
                generateAsserts(loc.concat(i), d + 1);
            }
        } else {
            console.log(`Assert A(${loc.join(', ')}) = ${expected.shift()}`);
        }
    }

    generateAsserts([], 0);

}

module.exports = {
    generateOutput: generateOutput
};
