
#include "Filter.h"

#include <iostream>
#include <cmath>

using namespace std;

int main(int argc, char **argv)
{
    vector<double> a { 1,5.75501989315662,16.326056867468,28.779190797823,34.2874379215653,28.137815126537,15.6064643257793,5.37874515231553,0.913800050254382,0.0,0.0 };
    vector<double> b { 0.0031954608137085,0.0180937089815597,0.0508407778575426,0.0895040074158415,0.107385387168148,0.0895040074158415,0.0508407778575426,0.0180937089815597,0.0031954608137085,0.0,0.0 };

    Filter f({ a, b });
    
    vector<double> in { 1, 2, 3, 4, 5, 6, 7, 8 };
    vector<double> out(8, 0.0);

    f.process(in.data(), out.data(), 8);

    vector<double> expected { 0.003195460813709, 0.006094690058282, 0.009370240771381, 0.012857578361690, 0.015328760300750, 0.019107809614909, 0.022257958968869, 0.024598034053011 };

    bool good = true;
    double thresh = 1e-14;
    
    for (int i = 0; i < 8; ++i) {
	if (fabs(out[i] - expected[i]) > thresh) {
	    cerr << "ERROR: out[" << i << "] (" << out[i]
		 << ") differs from expected[" << i << "] (" << expected[i]
		 << ") by " << out[i] - expected[i] << endl;
	    good = false;
	}
    }
    if (good) cerr << "SUCCESS" << endl;
}

