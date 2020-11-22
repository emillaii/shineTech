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
#ifndef MTF_RENDERER_FOCUS_H
#define MTF_RENDERER_FOCUS_H

#include "mtf_renderer.h"
#include "common_types.h"
#include "loess_fit.h"
#include "mtf_profile_sample.h"

#include <stdlib.h>
#include <vector>
using std::vector;
using std::pair;
using std::make_pair;

#include "focus_surface.h"
#include "distance_scale.h"
#include "mtf50_edge_quality_rating.h"
#include "bayer.h"
#include "ellipse.h"
#include "camera_draw.h"

class Mtf_renderer_focus : public Mtf_renderer {
  public:
    Mtf_renderer_focus(Distance_scale& distance_scale,
        const std::string& wdir, const std::string& prof_fname, 
        const cv::Mat& img, 
        bool lpmm_mode=false, double pixel_size=1.0) 
      :  zero(distance_scale.zero), 
         transverse(distance_scale.transverse), 
         longitudinal(distance_scale.longitudinal),
         wdir(wdir), prname(prof_fname),
         img(img), 
         lpmm_mode(lpmm_mode), pixel_size(pixel_size),
         distance_scale(distance_scale),
         psf(distance_scale.page_scale_factor),
         draw(img, distance_scale, 1.0, 220) {
         
    }
    
    void render(const vector<Block>&) {
       
        exit(1);
        return;
    }
    
    void render(const vector<Mtf_profile_sample>& samples, Bayer::bayer_t bayer = Bayer::NONE, 
        vector<Ellipse_detector>* ellipses = NULL, cv::Rect* dimension_correction = NULL) {
        Point2d centroid(0,0);
        
        cv::Mat merged = draw.rimg;
        initial_rows = draw.initial_rows;
        
        if (samples.size() < 100) {
            draw.fail_with_message(wdir + prname, string("Insufficient targets detected"));
            return;
        }
        
        if (!distance_scale.fiducials_found) {
            draw.fail_with_message(wdir + prname, string("Fiducials not found, probably incorrect chart type."));
            return;
        }
        
        double mean_y = 0;
        vector<Sample> data;
        double min_long = 1e50;
        double max_long = -1e50;
        double min_pix_long = 1e50;
        double max_pix_long = -1e50;
        for (size_t i=0; i < samples.size(); i++) {
            if (samples[i].quality > very_poor_quality) {
                Point2d d = samples[i].p - zero;
                Point2d coord(
                    d.x*longitudinal.x + d.y*longitudinal.y,
                    d.x*transverse.x + d.y*transverse.y
                );
                
                Point2d wc = distance_scale.estimate_world_coords(samples[i].p.x, samples[i].p.y);
                
                if (fabs(wc.y) < 20*psf && fabs(wc.x) < 180*psf) { 
                
                    mean_y += coord.y;
                    
                    double depth = 0;
                    distance_scale.estimate_depth_img_coords(samples[i].p.x, samples[i].p.y, depth);
                    coord.x = depth;
                    
                    // coord is now projected into the surface
                    data.push_back(Sample(coord.x, samples[i].mtf, 1.0, 1.0));
                    min_long = min(coord.x, min_long);
                    max_long = max(coord.x, max_long);
                    min_pix_long = min(min_pix_long, coord.x);
                    max_pix_long = max(max_pix_long, coord.x);
                }
            }
        }
        mean_y /= double(data.size());
        
        const int sh = 4;
        const double sgw[] = {-21/231.0, 14/231.0, 39/231.0, 54/231.0, 59/231.0, 54/231.0, 39/231.0, 14/231.0, -21/231.0};
        sort(data.begin(), data.end());
        
        // just pretend our samples are equally spaced
        vector<Sample> ndata;
        for (size_t i=sh; i < data.size() - sh; i++) {
            double val = 0;
            for (int w=-sh; w <= sh; w++) {
                val += sgw[w+sh] * data[i+w].y;    
            }
            
            
            if (fabs(val - data[i].y)/val < 0.04) {
                ndata.push_back(data[i]);
            }
            
        }
        data = ndata;
        
        Ratpoly_fit cf(data, 4, 2);
        VectorXd sol = rpfit(cf);
        
        // perform a few steps of IRLS
        double prev_err = 1e50;
        int dccount;
        for (int iter=0; iter < 50; iter++) {
            double errsum = 0;
            double wsum = 0;
            dccount = 0;
            for (size_t k=0; k < data.size(); k++) {
                double y = cf.rpeval(sol, data[k].x*cf.xsf)/cf.ysf;
                double e = fabs(y - data[k].y);
                errsum += e * data[k].yweight;
                wsum += data[k].yweight;
                data[k].yweight = 1.0; 
                if (e/y > 0.05) { // kill really poor outliers
                    data[k].yweight = 0;
                    dccount++;
                }
                
            }
            errsum /= wsum;
            sol = rpfit(cf);
            if (iter > 0 && (prev_err - errsum)/prev_err < 0.0001) {
                break;
            }
            prev_err = errsum;
        }
        double errsum = 0;
        double wsum = 0;
        for (size_t k=0; k < data.size(); k++) {
            double y = cf.rpeval(sol, data[k].x*cf.xsf)/cf.ysf;
            double e = fabs(y - data[k].y);
            errsum += e * data[k].yweight;
            wsum += data[k].yweight;
        }
        errsum /= wsum;
        
        // do a quick bootstrap to estimate some bounds on the focus peak
        vector<double> mc_peaks;
        const int n_mc = 30;
        for (int i=0; i < n_mc; i++) {
            vector<Sample> mc_data;
            for (size_t j=i; j < data.size(); j += 10) {
                mc_data.push_back(data[j]);
            }
            Ratpoly_fit mc_cf(mc_data, 4, 2);
            VectorXd mc_sol = rpfit(mc_cf);
            
            double mc_peak = mc_cf.peak(mc_sol);
            mc_peaks.push_back(mc_peak);
        }
        sort(mc_peaks.begin(), mc_peaks.end());
        double mc_p5 = mc_peaks[lrint(n_mc*0.05)];
        double mc_p95 = mc_peaks[lrint(n_mc*0.95)];
        
        string rp_name = wdir + ((bayer != Bayer::NONE) ? Bayer::to_string(bayer) + "_" : "") + "profile_points.txt";
        FILE* fraw = fopen(rp_name.c_str(), "wt");
        for (size_t i=0; i < data.size(); i++) {
            fprintf(fraw, "%lf %lf\n", data[i].x, data[i].y);
        }
        fclose(fraw);
        
        // now we can plot the reconstruction?
        double peak_y = 0;
        string cp_name = wdir + ((bayer != Bayer::NONE) ? Bayer::to_string(bayer) + "_" : "") + "profile_curve.txt";
        FILE* fout = fopen(cp_name.c_str(), "wt");
        for (double x=min_long; x < max_long; x += 1) {
            double y = cf.rpeval(sol, x*cf.xsf)/cf.ysf;
            fprintf(fout, "%lf %lf\n", x, y);
            
            if (y > peak_y) {
                peak_y = y;
            }
        }
        fclose(fout);
        
        double focus_peak = cf.peak(sol); // works on pre-stretched depth
        
        draw.chart_centre_marker();
        draw.camera_centre_marker(
            dimension_correction->width/2 - dimension_correction->x, 
            dimension_correction->height/2 - dimension_correction->y
        );
        
        if (ellipses) {
            cv::Scalar ellipse_col(200,200,0);
            for (auto e: *ellipses) {
                if (!e.valid) continue;
                Point2d prev(0,0);
                for (double theta=0; theta < 2*M_PI; theta += M_PI/64.0) {
                    double synth_x = e.major_axis * cos(theta);
                    double synth_y = e.minor_axis * sin(theta);
                    double rot_x = cos(e.angle)*synth_x - sin(e.angle)*synth_y + e.centroid_x;
                    double rot_y = sin(e.angle)*synth_x + cos(e.angle)*synth_y + e.centroid_y;

                    // clip to image size, just in case
                    rot_x = max(rot_x, 0.0);
                    rot_x = min(rot_x, (double)(merged.cols-1));
                    rot_y = max(rot_y, 0.0);
                    rot_y = min(rot_y, (double)(merged.rows-1));
                    
                    Point2d current(rot_x, rot_y);
                    if (theta > 0) {
                        cv::line(merged, prev, current, ellipse_col, 1, CV_AA);
                    }
                    
                    prev = current;
                }
            }
        }
        
        vector<Point2d> curve;
        min_pix_long = -merged.cols/2;
        max_pix_long = merged.cols/2;
        double mtf_peak_value = 0;
        double peak_wx = 0;
        // main MTF profile curve
        for (double x=min_pix_long; x < max_pix_long; x += 1) {
            double px = longitudinal.x * x  + longitudinal.y * mean_y + zero.x;
            double py = transverse.x * x + transverse.y *  mean_y + zero.y;
            
            double depth = 0;
            distance_scale.estimate_depth_img_coords(px, py, depth);
            
            if (depth > min_long && depth < max_long) {
                double mtf = cf.rpeval(sol, depth*cf.xsf)/cf.ysf;
                
                double world_y = psf*((mtf / peak_y) * 130 - 130);
                    
                Point2d wc = distance_scale.estimate_world_coords(px, py);
                
                if (mtf > mtf_peak_value) {
                    mtf_peak_value = mtf;
                    peak_wx = wc.x;
                }
                    
                Point2d proj_p = distance_scale.world_to_image(wc.x, world_y);
                curve.push_back(proj_p);
            }
        }
        if (cf.has_poles(sol)) {
            draw.fail_circle();
        } 
        draw.curve(curve, cv::Scalar(128, 128, 128), 6);
        draw.curve(curve, cv::Scalar(40, 90, 40), 3, cv::Scalar(40, 255, 40));
        
        curve.clear();
        for (double wy=-130*psf; wy < 130*psf; wy += 1) {
            Point2d proj_p = distance_scale.world_to_image(peak_wx, wy);
            curve.push_back(proj_p);
        }
        draw.curve(curve, cv::Scalar(255, 30, 30), 3);
        
        
        cv::Scalar axisdark = cv::Scalar(30, 140, 190);
        cv::Scalar axislight = cv::Scalar(40, 187, 255);
        
        curve.clear();
        for (double ystep=-135*psf; ystep <= 135*psf; ystep += 2) {
            curve.push_back(distance_scale.world_to_image(-180*psf, ystep));
        }
        draw.curve(curve, axisdark, 2, axisdark);
        curve.clear();
        for (double xstep=-180*psf; xstep <= 180*psf; xstep += 2) {
            curve.push_back(distance_scale.world_to_image(xstep, -135*psf));
        }
        draw.curve(curve, axisdark, 2, axislight);
        curve.clear();
        curve.push_back(distance_scale.world_to_image(-180*psf, -135*psf));
        
        // find a reasonable value for zmax that remains inside the image
        double zmax = 0;
        const double border = 35;
        for (; zmax > -200; zmax -= 2) {
            Point2d p = distance_scale.world_to_image(-180*psf, -135*psf, zmax*psf);
            if (p.x < border || p.y < border || p.x > img.cols - 1 - border || p.y > initial_rows - 1 - border) break;
        }
        
        curve.push_back(distance_scale.world_to_image(-180*psf, -135*psf, zmax*psf));
        draw.curve(curve, axisdark, 2, axisdark);
        
        int font = cv::FONT_HERSHEY_DUPLEX; 
        char tbuffer[1024];
        
        draw.text(-178*psf,  135*psf, 0, axisdark, "Y");
        draw.text( 180*psf, -133*psf, 0, axislight, "X");
        draw.text(-180*psf, -132*psf, zmax*psf, axisdark, "Z");
        
        cv::Scalar resultcolour(255, 255, 30);
        sprintf(tbuffer, "%.3lf (c/p)", mtf_peak_value);
        int tbaseline=0;
        cv::Size tsize = cv::getTextSize(tbuffer, font, 1.0, 2.5, &tbaseline);
        Point2d textpos = distance_scale.world_to_image(peak_wx, -20*psf);
        textpos -= Point2d(tsize.width/2.0, 0);
        cv::putText(merged, tbuffer, textpos, font, 1, CV_RGB(20, 20, 20), 2.5, CV_AA);
        cv::putText(merged, tbuffer, textpos, font, 1, resultcolour, 1, CV_AA);
        double prev_line_height = tsize.height;
        
        sprintf(tbuffer, "%.1lf mm", focus_peak);
        tsize = cv::getTextSize(tbuffer, font, 1.0, 2.5, &tbaseline);
        textpos = distance_scale.world_to_image(peak_wx, -20*psf);
        textpos -= Point2d(tsize.width/2.0, -prev_line_height*1.5);
        cv::putText(merged, tbuffer, textpos, font, 1, CV_RGB(20, 20, 20), 2.5, CV_AA);
        cv::putText(merged, tbuffer, textpos, font, 1, resultcolour, 1, CV_AA);
        
        // blank out the text region (again)
        rectangle(merged, Point2d(0, img.rows), Point2d(merged.cols, merged.rows), cv::Scalar::all(255), CV_FILLED);
        
        cv::Scalar red(30, 30, 200);
        cv::Scalar yellow(40, 187, 255);
        cv::Scalar black(0, 0, 0);
        cv::Scalar green(30, 200, 30);
        
        sprintf(tbuffer, "Focus peak at depth %.1lf mm [%.1lf,%.1lf] relative to chart origin.",focus_peak, mc_p5, mc_p95);
        int baseline = 0;
        cv::Size ts = cv::getTextSize(tbuffer, font, 1, 1, &baseline);
        draw.text(Point2d(50, initial_rows + ts.height*1.75), black, "%s", tbuffer);
        
        draw.text(Point2d(50, initial_rows + ts.height*2*1.75), black, "Estimated chart distance=%.2lf mm.", distance_scale.centre_depth);
        
        
        sprintf(tbuffer, "Bundle adjustment RMSE=%.3lf pixels (ideal < 1)", distance_scale.bundle_rmse);
        draw.text(Point2d(50, initial_rows + ts.height*3*1.75), black, "%s", tbuffer);
        
        ts = cv::getTextSize(tbuffer, font, 1, 1, &baseline);
        double col2 = ts.width + 150;
        cv::Scalar rmse_col = green;
        if (distance_scale.bundle_rmse < 1.5) {
            if (distance_scale.bundle_rmse > 0.7) {
                rmse_col = yellow;
            }
            draw.checkmark(Point2d(25, initial_rows + ts.height*3*1.75), rmse_col);
        } else {
            draw.crossmark(Point2d(35, initial_rows + ts.height*2.75*1.75), red);
        }
        
        double rpy = initial_rows + ts.height*4*1.75;
        double rpx = 50;
        draw.text(Point2d(rpx, rpy), black, "Chart z-angle=%.1lf degrees (ideal = 45)", distance_scale.get_normal_angle_z());
        
        cv::Scalar zang_col = green;
        if (fabs(distance_scale.get_normal_angle_z() - 45) < 10) {
            if (fabs(distance_scale.get_normal_angle_z() - 45) > 5) {
                zang_col = yellow;
            }
            draw.checkmark(Point2d(25, rpy), zang_col);
        } else {
            draw.crossmark(Point2d(35, rpy - 0.25*1.75*ts.height), red);
        }
        
        rpy = initial_rows + ts.height*5*1.75;
        draw.text(Point2d(rpx, rpy), black, "Chart y-angle=%.1lf degrees (ideal = 0)", distance_scale.get_normal_angle_y());
        
        cv::Scalar yang_col = green;
        if (fabs(distance_scale.get_normal_angle_y()) < 2) {
            if (fabs(distance_scale.get_normal_angle_y()) > 1) {
                yang_col = yellow;
            }
            draw.checkmark(Point2d(25, rpy), yang_col);
        } else {
            draw.crossmark(Point2d(35, rpy - 0.25*1.75*ts.height), red);
        }
        
        
        double white_clip = 0;
        double black_clip = 0;
        double overexposure = 0;
        exposure_checks(Point2d(180*psf, 135*psf), white_clip, black_clip, overexposure);
        
        rpx = col2;
        rpy = initial_rows + ts.height*3*1.75;
        if (white_clip > 5){
            sprintf(tbuffer, "Probable highlight clipping, severity=%.1lf%%", white_clip);
        } else {
            sprintf(tbuffer, "No highlight clipping detected");
        }
        draw.text(Point2d(rpx, rpy), black, "%s", tbuffer);
        
        cv::Scalar wc_col = green;
        if (white_clip < 10) {
            draw.checkmark(Point2d(rpx-25, rpy), wc_col);
        } else {
            draw.crossmark(Point2d(rpx-15, rpy - 0.25*1.75*ts.height), red);
        }
        
        rpy = initial_rows + ts.height*4*1.75;
        if (black_clip > 5){
            sprintf(tbuffer, "Probable shadow clipping, severity=%.1lf%%", black_clip);
        } else {
            sprintf(tbuffer, "No shadow clipping detected");
        }
        draw.text(Point2d(rpx, rpy), black, "%s", tbuffer);
        
        cv::Scalar bc_col = green;
        if (black_clip < 10) {
            draw.checkmark(Point2d(rpx-25, rpy), bc_col);
        } else {
            draw.crossmark(Point2d(rpx-15, rpy - 0.25*1.75*ts.height), red);
        }
        
        
        rpy = initial_rows + ts.height*5*1.75;
        if (overexposure > 0){
            draw.text(Point2d(rpx, rpy), black, "Probable overexposure, severity=%.1lf%%", overexposure);
            draw.crossmark(Point2d(rpx-15, rpy - 0.25*1.75*ts.height), red);
        } 
        
        
        imwrite(wdir + prname, merged);
        
    }

  private:
    
    VectorXd rpfit(Ratpoly_fit& cf, bool scale=true, bool refine=true) {
        const vector<Sample>& pts_row = cf.get_data();
        
        if (scale) {
            double xsf=0;
            double ysf=0;
            for (size_t i=0; i < pts_row.size(); i++) {
                xsf = max(xsf, fabs(pts_row[i].x));
                ysf = max(ysf, fabs(pts_row[i].y));
            }
            cf.xsf = xsf = 1.0/xsf;
            cf.ysf = ysf = 1.0/ysf;
        }
        
        int tdim = cf.dimension();
        MatrixXd cov = MatrixXd::Zero(tdim, tdim);
        VectorXd b = VectorXd::Zero(tdim);
        VectorXd a = VectorXd::Zero(tdim);
        
        VectorXd sol;
        
        for (int iter=0; iter < 1; iter++) {
            cov.setZero();
            b.setZero();
            a.setZero();
            
            for (size_t i=0; i < pts_row.size(); i++) {
                const Sample& sp = pts_row[i];
                double w = sp.weight * sp.yweight;
                a[0] = 1*w;
                double prod = sp.x * cf.xsf; // top poly
                for (int j=1; j <= cf.order_n; j++) {
                    a[j] = prod*w;
                    prod *= sp.x * cf.xsf;
                }
                prod = sp.x*cf.xsf; // bottom poly
                for (int j=1; j <= cf.order_m; j++) {
                    a[j+cf.order_n] = prod*w*sp.y*cf.ysf;
                    prod *= sp.x*cf.xsf;
                }
                
                for (int col=0; col < tdim; col++) { 
                    for (int icol=0; icol < tdim; icol++) { // build covariance of design matrix : A'*A
                        cov(col, icol) += a[col]*a[icol];
                    }
                    b[col] += cf.base_value*a[col]*sp.y*cf.ysf*w; // build rhs of system : A'*b
                }
            }
            
            for (int col=cf.order_n+1; col < cov.cols(); col++) {
                cov.col(col) = -cov.col(col);
            }
            
            sol = cov.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
        }
        
        // now perform non-linear optimization
        
        if (refine) {
            sol = cf.gauss_newton_armijo(sol);
        }
        
        return sol;
    }
    
    void exposure_checks(const Point2d& dims, double& white_clip, double& black_clip, double& overexposure) {
        
        vector<Point2d> corners { {-dims.x, -dims.y}, {dims.x, -dims.y}, {dims.x, dims.y}, {-dims.x, dims.y}};
        vector<cv::Point> roi;
        for (const auto& p: corners) {
            Point2d v = distance_scale.world_to_image(p.x, p.y);
            roi.push_back(cv::Point(v.x, v.y));
        }
        
        cv::Mat mask(img.rows, img.cols, CV_8UC1, cv::Scalar::all(0));
        cv::fillConvexPoly(mask, (const cv::Point*)roi.data(), roi.size(), cv::Scalar::all(255));
        
        // we could (in theory) mask out all the objects we used (ellipses and blocks)
        // that should take care of most of the gradients?
        
        cv::Rect bounds=cv::boundingRect(roi);
        
        bounds.x = max(0, bounds.x);
        bounds.y = max(0, bounds.y);
        bounds.width = min((img.cols - 1 - bounds.x), bounds.width);
        bounds.height = min((img.rows - 1 - bounds.y), bounds.height);
        
        //cv::Mat masked;
        //img.copyTo(masked, mask);
        
        // calculate histogram
        vector<uint32_t> histo(65536 + 1, 0);
        for (int row=bounds.y; row < (bounds.y + bounds.height); row++) {
            for (int col=bounds.x; col < (bounds.x + bounds.width); col++) {
                int val = img.at<uint16_t>(row, col);
                if (mask.at<uint8_t>(row, col) > 0) {
                    histo[val]++;
                }
            }
        }
        int last_nz = histo.size()-1;
        while (last_nz > 0 && histo[last_nz] == 0) last_nz--;
        
        #ifdef MDEBUG
        FILE* hfile = fopen("focus_histo.txt", "wt");
        size_t ch = 0;
        for (size_t j=0; j <= size_t(last_nz); j++) {
            if (histo[j] > 0) {
                ch += histo[j];
                fprintf(hfile, "%lu %u %lu\n", j, histo[j], ch);
            }
        }
        fclose(hfile);
        #endif
        
        vector<double> chisto(histo.size(), 0.0);
        chisto[0] = histo[0];
        for (size_t j=1; j < chisto.size(); j++) {
            chisto[j] = chisto[j-1] + histo[j];
        }
        
        // catch a histogram with an abnormally large count in the last bin
        white_clip = 0;
        if (chisto.back() - chisto[last_nz-1] > 0.05*chisto.back()) {
            white_clip = (chisto[last_nz] - chisto[last_nz-1])/chisto.back() * 100;
        }
        
        // catch a histogram with a bin within 5% of end with an abnormally large count
        int wc_sentinel = max(2.0, last_nz * 0.95);
        int wc_peak = last_nz;
        while (wc_peak > wc_sentinel) {
            if (chisto[wc_peak] - chisto[wc_peak-1] > 0.05*chisto.back()) {
                white_clip = (chisto[wc_peak] - chisto[wc_peak-1])/chisto.back() * 100;
            }
            wc_peak--;
        }
        
        overexposure = 0;
        // catch a histogram that has a > 70% weight in last 10% ....
        if ( (chisto.back() - chisto[65535*0.9]) > 0.7*histo.back() ) {
            overexposure = (chisto.back() - chisto[65535*0.9]) / (0.5*chisto.back()) * 100;
            overexposure = min(overexposure, 100.0);
        }
        
        int first_nz = 0;
        while (first_nz < (int)histo.size()-1 && histo[first_nz] == 0) first_nz++;
        
        vector<double> rhisto(histo.size(), 0.0);
        rhisto.back() = histo.back();
        for (size_t j=histo.size()-2; j > 0; j--) {
            rhisto[j] = rhisto[j+1] + histo[j];
        }
        rhisto[0] = rhisto[1] + histo[0];
        
        int p5 = first_nz+1;
        while (p5 < (int)histo.size()-1 && rhisto[p5] > 0.05*rhisto.front()) p5++;
        
        black_clip = 0;
        if (rhisto[first_nz] - rhisto[first_nz+1] > 0.0001*rhisto.front()) {
            black_clip = (rhisto[first_nz] - rhisto[first_nz+1])/(0.05*rhisto.front()) * 100;
            black_clip = min(black_clip, 100.0);
        }
        
        // compute integral image
        //cv::Mat temp;
        //masked.convertTo(temp, CV_32F);
        //integral(temp, masked, CV_32F);
        
        // homomorphic filtering tells us that we can subtract the log 
        // of the blurred version to obtain the "flat" lighting version
        // .... so the log of the blur is the slow-varying part
        // we want to see how uniform that is ...?
        
        // edges/nodata will be a problem (but we can construct a second mask?)
        
        
        // maybe we can simply blur a lot, and look at the
        // horizontal / vertical projections ? These should be flat?
    }
  
    Point2d& zero;
    Point2d& transverse;
    Point2d& longitudinal;
    std::string wdir;
    std::string prname;
    const cv::Mat& img;
    bool    lpmm_mode;
    double  pixel_size;
    Distance_scale& distance_scale;
    int initial_rows;
    
    double psf;
    
    Camera_draw draw;
};

#endif
