
#include "DCT.h"

#include <iostream>
#include <cmath>

using namespace std;

void check(string context, vector<double> got, vector<double> expected, bool &good)
{
//    cerr << "Checking " << context << "..." << endl;
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
    
    vector<double> in4 { 1, 2, 3, 5 };
    vector<double> out4(4), inv4(4);
    vector<double> expected4nu { 22.0, -8.1564, 1.4142, -1.2137 };
    vector<double> expected4u { 5.5, -2.8837, 0.5, -0.4291 };

    DCT d4(4);

    d4.forward(in4.data(), out4.data());
    check("out4", out4, expected4nu, good);

    d4.inverse(out4.data(), inv4.data());
    check("inverse4", inv4, in4, good);

    d4.forwardUnitary(in4.data(), out4.data());
    check("out4u", out4, expected4u, good);

    d4.inverseUnitary(out4.data(), inv4.data());
    check("inverse4u", inv4, in4, good);

    // do it again, just in case some internal state was modified in inverse
    
    d4.forwardUnitary(in4.data(), out4.data());
    check("out4u", out4, expected4u, good);

    d4.inverseUnitary(out4.data(), inv4.data());
    check("inverse4u", inv4, in4, good);
    
    vector<double> in5 { 1, 2, 3, 5, 6 };
    vector<double> out5(5), inv5(5);
    vector<double> expected5u { 7.6026, -4.1227, 0.3162, -0.0542, -0.3162 };

    DCT d5(5);

    d5.forwardUnitary(in5.data(), out5.data());
    check("out5u", out5, expected5u, good);

    d5.inverseUnitary(out5.data(), inv5.data());
    check("inverse5u", inv5, in5, good);
    
    if (good) {
	cerr << "Success" << endl;
	return 0;
    } else {
	return 1;
    }
}

