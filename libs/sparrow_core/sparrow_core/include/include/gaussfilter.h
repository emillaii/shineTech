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
#ifndef GAUSSFILTER_H
#define GAUSSFILTER_H

#include <vector>
using std::vector;

#include <stdio.h>

static double gauss9[19] = {
    0.00147728280397934,0.00379866200793248,0.00874062969790316,0.0179969888377294,0.0331590462642496,
    0.0546700248919979,0.0806569081730478,0.106482668507451,0.125794409230998,0.132980760133811,
    0.125794409230998,0.106482668507451,0.0806569081730478,0.0546700248919979,0.0331590462642496,
    0.0179969888377294,0.00874062969790316,0.00379866200793248,0.00147728280397934};

class Gaussfilter {
  public:
    // wrap-around filtering
    static void filter(double* data, size_t n) {
        const int w = 9;
        vector<double> result(n,0.0);
        for (size_t i=0; i < n; i++) {
            double wsum = 0;
            for (int j=-w; j <= w; j++) {
                wsum += gauss9[j+w];
                int eidx = int(i) + j;
                if (eidx < 0) eidx += n;
                eidx %= n;
                result[i] += gauss9[j+w] * data[eidx];
            }
            result[i] /= wsum;
        }
        for (size_t i=0; i < n; i++) {
            data[i] = result[i];
        }
    }
    
    // free boundary filtering
    static void filter(vector<double>& data) {
        size_t n = data.size();
        const int w = 9;
        vector<double> result(n,0.0);
        for (size_t i=0; i < n; i++) {
            double wsum = 0;
            for (int j=-w; j <= w; j++) {
                int eidx = int(i) + j;
                if (eidx >= 0 && eidx < int(n)-1) {
                    wsum += gauss9[j+w];
                    result[i] += gauss9[j+w] * data[eidx];
                }
            }
            result[i] /= wsum;
        }
        for (size_t i=0; i < n; i++) {
            data[i] = result[i];
        }
    }
};

#endif
