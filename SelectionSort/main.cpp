#include "cl_container.h"
#include <random>
#include <cmath>

using namespace std;

cl_container sps;

/*Based off of "https://mathbits.com/MathBits/CompSci/Arrays/Selection.htm"*/
void SelectionSort(vector <TYPE> &num)
{
	cout << "LOG: Running algorithm without parallel..." << endl;

	_timer timer;
	timer.start();

	int i, j, first, temp;
	int numLength = num.size();
	for (i = numLength - 1; i > 0; i--)
	{
		first = 0;                 // initialize to subscript of first element
		for (j = 1; j <= i; j++)   // locate smallest between positions 1 and i.
		{
			if (num[j] < num[first])
				first = j;
		}
		temp = num[first];   // Swap smallest found with element in position i.
		num[first] = num[i];
		num[i] = temp;
	}

	timer.stop();
	cout << "LOG: Finished non-parallel run. Elapsed time = " << timer.get_time() << endl;
}

int main() {
	// seed generator
	srand(time(NULL));

	// insert random data
	vector<TYPE> data;
	for (int k = 0; k < pow(2, 15); ++k)
		data.push_back(rand());

	sps.sort(data);

	SelectionSort(data);

	cin.get();
}