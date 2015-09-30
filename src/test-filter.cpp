
#include "Filter.h"

#include <iostream>
#include <cmath>

using namespace std;

int main(int argc, char **argv)
{
    // IIR
    
    vector<double> a { 1,5.75501989315662,16.326056867468,28.779190797823,34.2874379215653,28.137815126537,15.6064643257793,5.37874515231553,0.913800050254382,0.0,0.0 };
    vector<double> b { 0.0031954608137085,0.0180937089815597,0.0508407778575426,0.0895040074158415,0.107385387168148,0.0895040074158415,0.0508407778575426,0.0180937089815597,0.0031954608137085,0.0,0.0 };

    Filter f({ a, b });
    
    vector<double> in { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    vector<double> expected { 0.003195460813709, 0.006094690058282, 0.009370240771381, 0.012857578361690, 0.015328760300750, 0.019107809614909, 0.022257958968869, 0.024598034053011, 0.029106103380941, 0.031152166476509, 0.034424013713795, 0.038775350541015, 0.039924063374886, 0.044846280036012, 0.047614917256999, 0.049338485830505 };

    int n = expected.size();
    vector<double> out(n, 0.0);

    f.process(in.data(), out.data(), n);
    
    bool good = true;
    double thresh = 1e-12;
    
    for (int i = 0; i < n; ++i) {
	if (fabs(out[i] - expected[i]) > thresh) {
	    cerr << "ERROR: out[" << i << "] (" << out[i]
		 << ") differs from expected[" << i << "] (" << expected[i]
		 << ") by " << out[i] - expected[i] << endl;
	    good = false;
	}
    }

    // FIR

    b = { -1.5511e-18,-0.022664,1.047e-17,0.27398,0.49737,0.27398,1.047e-17,-0.022664,-1.5511e-18 };
    Filter ff({ {}, b });

    expected = { -1.5511e-18,-0.022664,-0.045328,0.20599,0.95467,1.9773,3,4,5,6,7,8,9,10,11,12 };

    n = expected.size();
    
    ff.process(in.data(), out.data(), n);

    thresh = 1e-4;
    
    for (int i = 0; i < n; ++i) {
	if (fabs(out[i] - expected[i]) > thresh) {
	    cerr << "ERROR: out[" << i << "] (" << out[i]
		 << ") differs from expected[" << i << "] (" << expected[i]
		 << ") by " << out[i] - expected[i] << endl;
	    good = false;
	}
    }
    
    if (good) {
	cerr << "Success" << endl;
	return 0;
    } else {
	return 1;
    }
}

