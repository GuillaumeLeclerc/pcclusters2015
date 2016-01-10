const fs = require('fs');
const data = fs.readFileSync('data', 'utf-8');
const lines = data.split('\n');
lines.pop();

const resultCount = lines.length / 6;

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

	if (statsParts[2] === 100) {
		results[statsParts[0]] = timings;
	}
}


for (var i = 1 ; i < 19; ++i) {
	console.log(results[i].join(','));
}
