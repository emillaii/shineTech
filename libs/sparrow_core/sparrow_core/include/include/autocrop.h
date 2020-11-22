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

#ifndef AUTOCROP_H
#define AUTOCROP_H    

#include "common_types.h"
#include <map>

using std::map;

class Autocropper { 
  public:
    Autocropper(cv::Mat& in_img) {
        const int border = 150;
        
        vector<double> int_rs(in_img.rows, 0);
        vector<double> int_cs(in_img.cols, 0);
        for (int r=4; r < in_img.rows-4; r+=2) {
            for (int c=4; c < in_img.cols-4; c+=2) {
                int_rs[r] += in_img.at<uint16_t>(r, c) / 65536.0;
                int_cs[c] += in_img.at<uint16_t>(r, c) / 65536.0;
                
                int_rs[r+1] = int_rs[r];
                int_cs[c+1] = int_cs[c];
            }
        }
        
        cv::Point_<int> bounds = otsu_bounds(int_rs, border);
        rstart = bounds.x;
        height = bounds.y;
        
        bounds = otsu_bounds(int_cs, border);
        cstart = bounds.x;
        width  = bounds.y;
        
        //logger.debug("start r/c: [%d %d], h/w: [%d %d]\n", rstart, cstart, height, width);
    }
    
    cv::Mat subset(const cv::Mat& X, cv::Rect* dimensions=NULL) {
        
        if (dimensions) {
            // what to add to cropped image coordinates to obtain original coordinates
            dimensions->x = cstart;
            dimensions->y = rstart;
            // original width and height
            dimensions->width = X.cols;
            dimensions->height = X.rows;
        }
        
        cv::Mat Y;
        X(cv::Rect(cstart, rstart, width, height)).copyTo(Y);
        return Y;
    }
    
    double otsu_threshold(const vector<double>& data) {
        const double binscale = 20.0;
        map<int, int> histo;
        double total = 0;
        for (size_t i=0; i < data.size(); i++ ) {
            int val = lrint(data[i] * binscale); 
        
            map<int, int>::iterator hi=histo.find(val);
            if (hi == histo.end()) {
                histo[val] = 0;
            }
            histo[val]++;
            total++;
        }
        double sum = 0;
        for (map<int, int>::iterator it=histo.begin(); it != histo.end(); it++) {
            sum += it->first * it->second;
        }
        // compute Otsu threshold
        double sum_b = 0;
        double w_b = 0;
        double max = 0;
        double thresh1 = 0;
        double thresh2 = 0;
        for (map<int, int>::iterator it=histo.begin(); it != histo.end(); it++) {
            w_b += it->second;
            double w_f = total - w_b;
            if (w_f == 0) break;
            
            sum_b += it->first * it->second;
            double m_b = sum_b / w_b;
            double m_f = (sum - sum_b) / w_f;
            double between = w_b * w_f * (m_b - m_f) * (m_b - m_f);
            if (between >= max) {
                thresh1 = it->first;
                if (between > max) {
                    thresh2 = it->first;
                }
                max = between;
            }
        }
        return ((thresh1 + thresh2) * 0.5) / binscale;
    }
    
    cv::Point_<int> otsu_bounds(const vector<double>& data, const int border) {
        
        double otsu = otsu_threshold(data);
        
        // if the left or right edge is above the threshold, then we probably have a bright edge
        // so we have to re-estimate the brightness threshold after compensating for that
        int dead_left = 0;
        while (data[dead_left] < otsu && dead_left < int(data.size()/4)) dead_left++;
        if (dead_left < 0.1*data.size()) {
            while (data[dead_left] > otsu && dead_left < int(data.size()/4)) dead_left++;
        } else {
            dead_left = 0;
        }
        int dead_right = data.size()-1;
        while (data[dead_right] < otsu && dead_right > int(3*data.size()/4)) dead_right--;
        if (dead_right > 0.9*data.size()) {
            while (data[dead_right] > otsu && dead_right > int(3*data.size()/4)) dead_right--;
        } else {
            dead_right = data.size()-1;
        }
        
        if (dead_left > 0 || dead_right < (int)data.size()-1) {
            vector<double> dcopy(data);
            sort(dcopy.begin(), dcopy.end());
            double median = dcopy[dcopy.size()/2];
            // replace values above threshold with median?
            dcopy = data;
            for (auto& d: dcopy) {
                if (d > otsu) {
                    d = median;
                }
            }
            otsu = otsu_threshold(dcopy);
            //logger.debug("autocrop had to use a deadzone because the image borders were bright\n");
        }
        
        int upper = dead_left;
        int lower = dead_right;
        
        for (int i=dead_left; i < (int)data.size(); i++) {
            if (data[i] > otsu) {
                upper = i;
            }
        }
        for (int i=dead_right; i > 0; i--) {
            if (data[i] > otsu) {
                lower = i;
            }
        }
        
        int even_start = lower - border;
        even_start += even_start % 2;
        even_start  = max(0, even_start);
        int even_end = min(int(data.size()-1), upper + border);
        even_end -= even_end % 2;
        int ewidth = (even_end - even_start);
        
        return cv::Point_<int>(even_start, ewidth);
    }
    
    int rstart;
    int cstart;
    int width;
    int height;
    
};    
    
#endif
    
