const nullbyte = require('./build/Release/nullbyte');

function patch(processId, patterns) {
   return nullbyte.patch(processId, patterns);
}

module.exports = { patch };