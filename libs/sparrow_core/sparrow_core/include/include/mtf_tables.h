/*
Copyright 2011 Frans van den Bergh. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Frans van den Bergh ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Frans van den Bergh OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of the Council for Scientific and Industrial Research (CSIR).
*/
#ifndef MTFTABLES_H
#define MTFTABLES_H

#include <vector>
using std::vector;
#include <cmath>

#include "mtf_core.h"

class Mtf_correction;

extern Mtf_correction* global_mtf_correction_instance;


class Mtf_correction {

  public:
    Mtf_correction(void) : w(NYQUIST_FREQ*4, 0.0) {
        w[0] = 1.0;
        for (int i=1; i < NYQUIST_FREQ*4; i++) {
            double dc_x = 2*M_PI*i/double(NYQUIST_FREQ*2*8); // 8 is correction factor for 8x oversample discrete derivative
            double lp_x = M_PI*i/double(NYQUIST_FREQ*2);
            double expcor = (sdev*sdev)/(sdev*sdev + 4*lp_x*lp_x);
            w[i] = (sin(dc_x)/dc_x) * expcor;
        }
    }
    
    static Mtf_correction* get_instance(void) {
        return global_mtf_correction_instance;
    }
    
    vector<double> w;
    
    static const double sdev;
private:

    
};

const double savitsky_golay[6][15] = {
  {0,0,0,0,0,-0.085714285714286,0.342857142857143,0.485714285714286,0.342857142857143,-0.085714285714286,0,0,0,0,0},
  {0,0,0,0,-0.095238095238095,0.142857142857143,0.285714285714286,0.333333333333333,0.285714285714286,0.142857142857143,-0.095238095238095,0,0,0,0},
  {0,0,0,-0.090909090909091,0.060606060606061,0.168831168831169,0.233766233766234,0.255411255411255,0.233766233766234,0.168831168831169,0.060606060606061,-0.090909090909091,0,0,0},
  {0,0,-0.083916083916084,0.020979020979021,0.102564102564103,0.160839160839161,0.195804195804196,0.207459207459208,0.195804195804196,0.160839160839161,0.102564102564103,0.020979020979021,-0.083916083916084,0,0},
  {0,-0.076923076923077,0.000000000000000,0.062937062937063,0.111888111888112,0.146853146853147,0.167832167832168,0.174825174825175,0.167832167832168,0.146853146853147,0.111888111888112,0.062937062937063,0.000000000000000,-0.076923076923077,0},
  {-0.070588235294118,-0.011764705882353,0.038009049773756,0.078733031674208,0.110407239819004,0.133031674208145,0.146606334841629,0.151131221719457,0.146606334841629,0.133031674208145,0.110407239819004,0.078733031674208,0.038009049773756,-0.011764705882353,-0.070588235294118}
};

#endif // MTFTABLES_H
