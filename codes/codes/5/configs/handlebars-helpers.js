module.exports = {
    lengthOfList: (list = []) => list.length,
    eq: (val1, val2) => val1 === val2,
    dataString: (isoString) => new Date(isoString).toLocaleString(),
};