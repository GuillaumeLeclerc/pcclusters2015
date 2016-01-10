const fs = require('fs');
const data = fs.readFileSync('data', 'utf-8');
const lines = data.split('\n');
lines.pop();

const resultCount = lines.length / 6;

function sortNumber(a,b) {
	    return a - b;
}

const results = {};

for (var i = 0 ; i < resultCount; ++i) {
	var rawStats = lines.shift();
	var statsParts = rawStats.split(' - ').map(function(x) {
		return parseInt(x);
	});

	var timings = [];
	for (var j = 0 ; j < 5; j++) {
		timings.push(parseFloat(lines.shift()));
	}

	if ((statsParts[2] === 10000 || statsParts[2] === 10000) && statsParts[1] === 1) {
		results[statsParts[0]] = timings;
	}
}

var keys = Object.keys(results)
keys.sort(sortNumber);
keys.forEach(function(x) {
	console.log(results[x].join(','));
});

