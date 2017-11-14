#include "cl_container.h"
#include <random>

using namespace std;

cl_container sps;

int main() {
	// seed generator
	srand(time(NULL));

	// insert random data
	vector<TYPE> data;
	for (int k = 0; k < 16; ++k)
		data.push_back(rand() % 4096);

	sps.sort(data);

	cin.get();
}