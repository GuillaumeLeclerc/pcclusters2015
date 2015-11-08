#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>

long syracuse_next(long n) {
	if(n % 2 == 0) return n/2;
	return 3*n + 1;
}

int main(int argc, const char** argv) {
	int i = 1;
	long max = 100000;

	std::vector<int> values;
	values.resize(max, -1);
	if (argc > 1) {
		max = atoi(argv[1]);
	}

	while (i < max) {
		i++;
		int seq = 0;
		long v = i;
		int steps = 0;
		while (v != 1) {
			steps++;
			if (v < values.size() && values[v] != -1) {
				seq += values[v];
				break;
			}
			v = syracuse_next(v);
			seq++;
		}
		if (i >= values.size()) {
			values.resize(i + 1, -1);
		}
		if (i < max * 0.01) {
			values[i] = seq;
		} else {
			std::cout << i << '\t' << steps << '\t' << seq  << std::endl;
		}
	}

	return 0;
}
