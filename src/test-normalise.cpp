
#include "Normalise.h"

#include <iostream>
#include <cmath>

using namespace std;

void check(string context, vector<double> got, vector<double> expected, bool &good)
{
    double thresh = 1e-4;
    for (int i = 0; i < int(got.size()); ++i) {
	if (fabs(got[i] - expected[i]) > thresh) {
	    cerr << "ERROR: " << context << "[" << i << "] (" << got[i]
		 << ") differs from expected " << expected[i] << endl;
	    good = false;
	}
    }
}

int main(int argc, char **argv)
{
    bool good = true;
    
    vector<double> in { -1, 1.5, 3, 5 };

    vector<double> out, expected;

    out = Normalise::normalise(in, 1);
    expected = { -0.095238, 0.142857, 0.285714, 0.476190 };
    check("L1", out, expected, good);
    
    out = Normalise::normalise(in, 2);
    expected = { -0.16385, 0.24577, 0.49154, 0.81923 };
    check("L2", out, expected, good);
    
    out = Normalise::normalise(in, 3);
    expected = { -0.18561, 0.27842, 0.55684, 0.92807 };
    check("L3", out, expected, good);

    in = { 0, 0, 0, 0 };

    out = Normalise::normalise(in, 1);
    expected = { 0.25, 0.25, 0.25, 0.25 };
    check("L1_zero", out, expected, good);
    
    out = Normalise::normalise(in, 2);
    expected = { 0.5, 0.5, 0.5, 0.5 };
    check("L2_zero", out, expected, good);
    
    out = Normalise::normalise(in, 3);
    expected = { 0.62996, 0.62996, 0.62996, 0.62996 };
    check("L3_zero", out, expected, good);

    if (good) {
	cerr << "Success" << endl;
	return 0;
    } else {
	return 1;
    }
}


