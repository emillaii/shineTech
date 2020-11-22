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
#ifndef GAMMA_H
#define GAMMA_H

class Gamma {

public:
    Gamma(void) {
        for (size_t i=0; i < 256; i++) {
            _gamma_lut[i] = uint16_t(lrint(_linearize_gamma(i)*65535));
        }
    }
    
    inline uint16_t linearize_gamma(unsigned char x)  const {
        return _gamma_lut[x];
    }

private:

    inline double _linearize_gamma(unsigned char x) const {
        const double C_linear = 0.0404482;
        const double S_linear = 12.9232102;
        const double SRGB_a = 0.055;

        double y = x / 255.0;

        if (y < C_linear) {
            return y / S_linear;
        }
        return pow((y + SRGB_a) / (1.0 + SRGB_a), 2.4);
    }

    uint16_t _gamma_lut[256];    
    
};    
    
#endif // GAMMA_H
