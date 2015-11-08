#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>

long syracuse_next(long n) {
	if(n % 2 == 0) return n/2;
	return 3*n + 1;
}

int main(int argc, const char** argv) {
	std::cout << "Hello world" << std::endl;

	int i = 1;
	long max = 100000;

	std::vector<int> values;
	values.resize(max, -1);
	if (argc > 1) {
		max = atoi(argv[1]);
		std::cout << "Target: " << max << std::endl;
	}

	while (i < max) {
		i++;
		int seq = 0;
		long v = i;
		while (v != 1) {
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
		values[i] = seq;
		//std::cout << i << ":" << seq << std::endl;
	}

	return 0;
}
