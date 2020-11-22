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
#ifndef MTF_RENDERER_ANNOTATE_H
#define MTF_RENDERER_ANNOTATE_H

#include "mtf_renderer.h"
#include "common_types.h"
#include "include/srgb_render.h"

#include <opencv2/highgui/highgui.hpp>

class Mtf_renderer_annotate : public Mtf_renderer {
  public:
    Mtf_renderer_annotate(const cv::Mat& in_img, const std::string& fname,
      bool lpmm_mode, double pixel_size) 
      : img(in_img), ofname(fname), lpmm_mode(lpmm_mode), pixel_size(pixel_size) {
      
          out_img = Srgb_render::linear_to_sRGB(in_img);
    }
    
    void render(const vector<Block>& blocks) {
        for (size_t i=0; i < blocks.size(); i++) {
            for (size_t k=0; k < 4; k++) {
                double val = blocks[i].get_mtf50_value(k);
                if (val > 0) {
                    Point2d cent = blocks[i].get_edge_centroid(k);
                    if (cent.x > 1 && cent.y > 1) {
                        write_number(out_img, lrint(cent.x), lrint(cent.y), val, blocks[i].get_quality(k));
                    }
                }
            }
        }    
        
        imwrite(ofname, out_img);
    }
    
    void write_number(cv::Mat& img, int px, int py, double val, double quality) {
        char buffer[10];
        
        if (val < 1) {
            sprintf(buffer, "%.2lf", val);
        } else {
            sprintf(buffer, "N/A");
        }
        
        int baseline = 0;
        
        cv::Size ts = cv::getTextSize(buffer, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
        cv::Point to(-ts.width/2,  ts.height/2);
        to.x += px;
        to.y += py;
        
        cv::putText(img, buffer, to, 
            cv::FONT_HERSHEY_SIMPLEX, 0.5, 
            CV_RGB(20, 20, 20), 2.5, CV_AA
        );
        
        cv::Scalar col = CV_RGB(0, 255, 255);
        if (quality < 0.8) {
            col = CV_RGB(255, 255, 0);
        }
        if (quality <= 0.2 || val == 1.0) {
            col = CV_RGB(255, 0, 0);
        }
        
        cv::putText(img, buffer, to, 
            cv::FONT_HERSHEY_SIMPLEX, 0.5, 
            col, 1, CV_AA
        );
        
    }
    
    const cv::Mat& img;
    cv::Mat out_img;
    string ofname;
    bool    lpmm_mode;
    double  pixel_size;
};

#endif
