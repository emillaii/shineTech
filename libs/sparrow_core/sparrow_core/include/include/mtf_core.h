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
#ifndef MTF_CORE_H
#define MTF_CORE_H

#include "include/component_labelling.h"
#include "include/gradient.h"
#include "include/block.h"
#include "include/rectangle.h"
#include "include/ellipse.h"
#include "include/edge_record.h"
#include "include/loess_fit.h"
#include "include/afft.h"
#include "include/mtf_profile_sample.h"
#include "include/bayer.h"
#include "include/esf_sampler.h"


#include "include/esf_sampler_line.h"
#include "include/esf_sampler_deferred.h"
#include "include/esf_sampler_quad.h"
#include "include/esf_sampler_piecewise_quad.h"
#include "include/undistort.h"
#include "sfr.h"


#include <map>
using std::map;

#include <opencv2/imgproc/imgproc.hpp>

typedef vector<Block> block_vector;

// global constants for ESF-fourier MTF method
// TODO: these can be dynamic parameters, with some effort
const double max_dot = 28;
const int SAMPLES_PER_PIXEL = 32;
const size_t FFT_SIZE = int(16)*SAMPLES_PER_PIXEL;
const int NYQUIST_FREQ = FFT_SIZE/16;
//const int NYQUIST_FREQ = 8;

class Mtf_core {
  public:
    

    Mtf_core(const Component_labeller& in_cl, const Gradient& in_g, 
             const cv::Mat& in_img, const cv::Mat& in_bayer_img, 
             std::string bayer_subset, std::string cfa_pattern_name,
             std::string esf_sampler_name, Undistort* undistort=nullptr, int border_width=0)
             
      : cl(in_cl), g(in_g), img(in_img), bayer_img(in_bayer_img), absolute_sfr(false),
        snap_to(false), snap_to_angle(0), sfr_smoothing(true),
        sliding(false), samples_per_edge(0), border_width(border_width), find_fiducials(false),
        undistort(undistort), ridges_only(false) {

        bayer = Bayer::from_string(bayer_subset);
        Bayer::cfa_pattern_t cfa_pattern = Bayer::from_cfa_string(cfa_pattern_name);

        switch (Esf_sampler::from_string(esf_sampler_name)) {
        case Esf_sampler::LINE:
            esf_sampler = new Esf_sampler_line(max_dot, Bayer::to_cfa_mask(bayer, cfa_pattern), border_width);
            break;
        case Esf_sampler::QUAD:
            esf_sampler = new Esf_sampler_quad(max_dot, Bayer::to_cfa_mask(bayer, cfa_pattern), border_width);
            break;
        case Esf_sampler::PIECEWISE_QUAD:
            esf_sampler = new Esf_sampler_piecewise_quad(max_dot, Bayer::to_cfa_mask(bayer, cfa_pattern), border_width);
            break;
        case Esf_sampler::DEFERRED:
            if (!undistort) { // hopefully we stop it before this point ...
                exit(-1);
            }
            esf_sampler = new Esf_sampler_deferred(undistort, max_dot, Bayer::to_cfa_mask(bayer, cfa_pattern), border_width);
            break;
        };
      
        for (Boundarylist::const_iterator it=cl.get_boundaries().begin(); it != cl.get_boundaries().end(); ++it) {
            valid_obj.push_back(it->first);
        }
        
        cv::Mat temp;
        in_img.convertTo(temp, CV_8U, 256.0/16384.0);
        cv::cvtColor(temp, od_img, CV_GRAY2RGB);
    }
    
    ~Mtf_core(void) {
        //cv::imwrite(string("detections.png"), od_img);
    }
    
    size_t num_objects(void) {
        return valid_obj.size();
    }
    
    void search_borders(const Point2d& cent, int label, sfr::EdgeFilter filter);
    bool extract_rectangle(const Point2d& cent, int label, Mrectangle& rect);
    double compute_mtf(Edge_model& edge_model, const map<int, scanline>& scanset, 
                       double& poor, 
                       vector<double>& sfr, vector<double>& esf, 
                       bool allow_peak_shift = false);
    
    vector<Block>& get_blocks(void) {
        // make a copy into an STL container if necessary
        if (detected_blocks.size() == 0) {
            for (map<int,Block>::const_iterator it=shared_blocks_map.begin();
                 it != shared_blocks_map.end(); ++it) {
                
                bool allzero = true;
                for (int k=0; k < 4 && allzero; k++) {
                    if (fabs(it->second.get_mtf50_value(k)) > 1e-6) {
                        allzero = false;
                    }
                }
                
                if (it->second.valid && !allzero) { 
                    detected_blocks.push_back(it->second);
                }
            }
        }
        return detected_blocks;
    }
    
    vector<Mtf_profile_sample>& get_samples(void) {
        return samples;
    }
    
    void set_absolute_sfr(bool val) {
        absolute_sfr = val;
    }
    
    void set_sfr_smoothing(bool val) {
        sfr_smoothing = val;
    }
    
    void set_sliding(bool val) {
        sliding = val;
        find_fiducials = true;
    }
    
    void set_samples_per_edge(int s) {
        samples_per_edge = s;
    }
    
    void set_snap_angle(double angle) {
        snap_to = true;
        snap_to_angle = angle;
    }
    
    void set_find_fiducials(bool val) {
        find_fiducials = val;
    }
    
    void set_ridges_only(bool b) {
        ridges_only = b;
    }

    void use_full_sfr(void) {
        mtf_width = 4 * NYQUIST_FREQ;
    }
    
    void set_mtf_contrast(double contrast) {
        mtf_contrast = std::max(0.01, std::min(contrast, 0.9));
    }
    
    double get_mtf_contrast(void) const {
        return mtf_contrast;
    }
    
    double process_image_as_roi(void);
    
    const Component_labeller& cl;
    const Gradient&           g;
    const cv::Mat&            img;
    const cv::Mat&            bayer_img;
    Bayer::bayer_t bayer;
    
    AFFT<512> afft; // FFT_SIZE = 512 ??
    vector<int> valid_obj;
    
    vector<Block> detected_blocks;  
    map<int, Block> shared_blocks_map;
    vector<Point2d> solid_ellipses;
    vector<Ellipse_detector> ellipses;
    
    vector<Mtf_profile_sample> samples;
    
    cv::Mat od_img;
    
  private:
    bool absolute_sfr;
    bool snap_to;
    double snap_to_angle;
    bool sfr_smoothing;
    bool sliding;
    int samples_per_edge;
    int border_width;
    bool find_fiducials;
    Undistort* undistort = nullptr;
    bool ridges_only;
    size_t mtf_width = 2 * NYQUIST_FREQ;
    Esf_sampler* esf_sampler = nullptr;
    double mtf_contrast = 0.5; // target MTF contrast, e.g., 0.5 -> MTF50
    
    void process_with_sliding_window(Mrectangle& rrect);
    bool homogenous(const Point2d& cent, int label, const Mrectangle& rrect) const;
};

#endif
