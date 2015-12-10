#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>


struct computationRequest {
	long value;
	int depth;
};

struct computationResult {
	int value;
	int steps;
};

long syracuse_next(long n) {
	if(n % 2 == 0) return n/2;
	return 3*n + 1;
}

int main(int argc, const char** argv) {
	int i = 1;
	long max = 100000;
	if (argc > 1) {
		max = atoi(argv[1]);
		std::cout << "Target: " << max << std::endl;
	}

	std::vector<int> values;
	std::vector<computationRequest> *toCompute = new std::vector<computationRequest>();

	values.resize(max, -1);
	values[1] = 1;
	int count = 1;
	computationRequest start = { 1, 0};

	toCompute->push_back(start);


	int computed = 0;
	while (toCompute->size() > 0) {
		std::vector<computationRequest> *next = new std::vector<computationRequest>();
		for (int j = 0 ; j < toCompute->size() ; ++j) {
			computed++;
			computationRequest a = toCompute->at(j);
			if (a.value < values.size()) {
				values[a.value] = a.depth;
				++count;
			}
			computationRequest b = {2*a.value, a.depth + 1};
			computationRequest c = {(a.value- 1) / 3, a.depth + 1};
			if (b.value <= max && b.value > 1) {
				next->push_back(b);
			}
			if (c.value <= max && c.value > 1 && (a.value - 1) % 3 == 0 && c.value % 2 == 1) {
				next->push_back(c);
			}
		}
		delete toCompute;
		toCompute = next;
	}

	std::cout << computed * 100.0 / max << std::endl;
	exit(0);

	int unknown = 0;
	for (int j = 0 ; j < values.size() ; ++j) {
		if(values[j] == -1) unknown++;
	}
	std::cout << unknown << std::endl;
	while (i < max) {
		i++;
		int seq = 0;
		long v = i;
		int steps = 0;
		while (v != 1) {
			++steps;
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
		//std::cout << i << ":" << steps << std::endl;
		values[i] = seq;
		//std::cout << i << ":" << seq << std::endl;
	}

	return 0;
}
