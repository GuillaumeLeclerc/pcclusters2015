var fs = require('fs');

var lines = fs.readFileSync('tempResult', {encoding: 'utf-8'}).split('\n');

var data = {};

for (var i = 0; i < lines.length; i += 2) {
	if (lines[i].length > 0) {
		var header = lines[i];
		var time = parseFloat(lines[i + 1]);
		data[header] = time;
	}
}

var rounds = [1, 10, 100, 1000, 10000];

for (var ri = 0; ri < rounds.length; ri++) {
	var round = rounds[ri];
	var line = "";
	for (var node = 1 ; node < 16; node++) {
		var tt = node + ' - 1 - ' + round;
		if (data[tt]) {
			line += data[tt];
		}
		line += ";";
	}
	console.log(line);
}
