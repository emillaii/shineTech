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
#ifndef MTF_RENDERER_GRID_H
#define MTF_RENDERER_GRID_H

#include "mtf_renderer.h"
#include "common_types.h"
#include "gaussfilter.h"
#include <Eigen/Dense>
#include "mtf50_edge_quality_rating.h"
#include "mtf_profile_sample.h"

// Moreland's diverging colour map:
// Moreland, K., Diverging Color Maps for Scientific Visualization, In Proceedings of 
// the 5th International Symposium on Visual Computing, December 2009.

const std::string diverging_palette = 
"set palette defined(\
0       0.2314  0.2980  0.7529,\
0.03125 0.2667  0.3529  0.8000,\
0.0625  0.3020  0.4078  0.8431,\
0.09375 0.3412  0.4588  0.8824,\
0.125   0.3843  0.5098  0.9176,\
0.15625 0.4235  0.5569  0.9451,\
0.1875  0.4667  0.6039  0.9686,\
0.21875 0.5098  0.6471  0.9843,\
0.25    0.5529  0.6902  0.9961,\
0.28125 0.5961  0.7255  1.0000,\
0.3125  0.6392  0.7608  1.0000,\
0.34375 0.6824  0.7882  0.9922,\
0.375   0.7216  0.8157  0.9765,\
0.40625 0.7608  0.8353  0.9569,\
0.4375  0.8000  0.8510  0.9333,\
0.46875 0.8353  0.8588  0.9020,\
0.5     0.8667  0.8667  0.8667,\
0.53125 0.8980  0.8471  0.8196,\
0.5625  0.9255  0.8275  0.7725,\
0.59375 0.9451  0.8000  0.7255,\
0.625   0.9608  0.7686  0.6784,\
0.65625 0.9686  0.7333  0.6275,\
0.6875  0.9686  0.6941  0.5804,\
0.71875 0.9686  0.6510  0.5294,\
0.75    0.9569  0.6039  0.4824,\
0.78125 0.9451  0.5529  0.4353,\
0.8125  0.9255  0.4980  0.3882,\
0.84375 0.8980  0.4392  0.3451,\
0.875   0.8706  0.3765  0.3020,\
0.90625 0.8353  0.3137  0.2588,\
0.9375  0.7961  0.2431  0.2196,\
0.96875 0.7529  0.1569  0.1843,\
1       0.7059  0.0157  0.1490\
)";

#include <set>
using std::set;

class Mtf_renderer_grid : public Mtf_renderer {
  public:
    Mtf_renderer_grid(
        const std::string& img_filename,
        const std::string& wdir, const std::string& fname, 
        const std::string& gnuplot_binary, 
        const cv::Mat& img, int gnuplot_width,
        bool lpmm_mode, double pixel_size,
        double in_zscale)
      :  Mtf_renderer(img_filename),
         wdir(wdir), fname(fname), 
         gnuplot_binary(gnuplot_binary), img_y(img.rows), img_x(img.cols),
         img(img), lpmm_mode(lpmm_mode), pixel_size(pixel_size),
         gnuplot_failure(false), gnuplot_warning(true),
         m_lower(0), m_upper(0), gnuplot_width(gnuplot_width) {

        const int coarse_grid_size = 40;
        const int fine_grid_size = 200;
        if (img.rows > img.cols) {
            grid_y_coarse = coarse_grid_size;
            grid_x_coarse = coarse_grid_size * img.cols / img.rows;
            grid_y_fine = fine_grid_size;
            grid_x_fine = fine_grid_size * img.cols / img.rows;
        } else {
            grid_x_coarse = coarse_grid_size;
            grid_y_coarse = coarse_grid_size * img.rows / img.cols;
            grid_x_fine = fine_grid_size;
            grid_y_fine = fine_grid_size * img.rows / img.cols;
        }
        
        zscale = std::max(0.0, std::min(in_zscale, 1.0));
    }
    
    void set_gnuplot_warning(bool gnuplot) {
        gnuplot_warning = gnuplot;
    }
    
    void render(const vector<Block>& blocks) {

        // first check if we have enough good data to generate a plot
        vector<double> allvals;
        for (size_t i=0; i < blocks.size(); i++) {
            for (size_t k=0; k < 4; k++) {
                double val = blocks[i].get_mtf50_value(k);
                if (val > 0 && val < 1.0 && blocks[i].get_quality(k) > very_poor_quality) {
                    allvals.push_back(val);
                }
            }
        }

        if (allvals.size() < 20) {
            return;
        }

        sort(allvals.begin(), allvals.end());
        m_upper = allvals[97*allvals.size()/100];
        m_lower = allvals[5*allvals.size()/100];

        cv::Mat grid_mer_coarse(grid_y_coarse, grid_x_coarse, CV_32FC1, 0.0);
        cv::Mat grid_mer_fine(grid_y_fine, grid_x_fine, CV_32FC1, 0.0);
        extract_mtf_grid(MERIDIONAL, grid_mer_coarse, grid_mer_fine, blocks, m_upper);
        cv::Mat grid_sag_coarse(grid_y_coarse, grid_x_coarse, CV_32FC1, 0.0);
        cv::Mat grid_sag_fine(grid_y_fine, grid_x_fine, CV_32FC1, 0.0);
        extract_mtf_grid(SAGITTAL, grid_sag_coarse, grid_sag_fine, blocks, m_upper);
        
        double zmax = 0;
        double zmin = 1e30;
        FILE* file = fopen((wdir+fname).c_str(), "wt");
        fprintf(file, "#coarse meridional grid\n");
        for (int y=0; y < grid_mer_coarse.rows; y++) {
            for (int x=0; x < grid_mer_coarse.cols; x++) {
                fprintf(file, "%lf %lf %.5lf\n", 
                    x*img.cols/grid_mer_coarse.cols/pixel_size, 
                    y*img.rows/grid_mer_coarse.rows/pixel_size, 
                    grid_mer_coarse.at<float>(y,x)*pixel_size
                );
            }
            fprintf(file, "\n");
        }
        fprintf(file, "\n\n");
        fprintf(file, "#coarse sagittal grid\n");
        for (int y=0; y < grid_sag_coarse.rows; y++) {
            for (int x=0; x < grid_sag_coarse.cols; x++) {
                fprintf(file, "%lf %lf %.5lf\n", 
                    x*img.cols/grid_sag_coarse.cols/pixel_size, 
                    y*img.rows/grid_sag_coarse.rows/pixel_size, 
                    grid_sag_coarse.at<float>(y,x)*pixel_size
                );
            }
            fprintf(file, "\n");
        }
        fprintf(file, "\n\n");
        
        fprintf(file, "#fine meridional grid\n");
        for (int y=0; y < grid_mer_fine.rows; y++) {
            for (int x=0; x < grid_mer_fine.cols; x++) {
                fprintf(file, "%lf %lf %.5lf\n", 
                    x*img.cols/grid_mer_fine.cols/pixel_size, 
                    y*img.rows/grid_mer_fine.rows/pixel_size, 
                    grid_mer_fine.at<float>(y,x)*pixel_size
                );
                zmax = std::max(zmax, (double)grid_mer_fine.at<float>(y,x)*pixel_size);
                zmin = std::min(zmin, (double)grid_mer_fine.at<float>(y,x)*pixel_size);
            }
            fprintf(file, "\n");
        }
        fprintf(file, "\n\n");
        
        fprintf(file, "#fine sagittal grid\n");
        for (int y=0; y < grid_sag_fine.rows; y++) {
            for (int x=0; x < grid_sag_fine.cols; x++) {
                fprintf(file, "%lf %lf %.5lf\n", 
                    x*img.cols/grid_sag_fine.cols/pixel_size, 
                    y*img.rows/grid_sag_fine.rows/pixel_size, 
                    grid_sag_fine.at<float>(y,x)*pixel_size
                );
                zmax = std::max(zmax, (double)grid_sag_fine.at<float>(y,x)*pixel_size);
                zmin = std::min(zmin, (double)grid_sag_fine.at<float>(y,x)*pixel_size);
            }
            fprintf(file, "\n");
        }
        
        fclose(file);
        
        // TODO: sort out the mess (basename, etc)
        string img_name_clean;
        for (size_t i=0; i < img_filename.length(); i++) {
            if (img_name_clean.length() > 0 && img_name_clean[img_name_clean.length()-1] == '\\' &&
                img_filename[i] == '\\') {
                // skip
            } else {
                img_name_clean.push_back(img_filename[i]);
            }
        }
        
        double span = zmax - zmin;
        zmax = zmax + 0.05*span;
        zmin = zmin - 0.05*span;
        zmin = std::max(zmin, 0.0);
        
        // adapt zmin according to zscale setting
        // zscale = 0 -> use zmin=0
        // zscale = 1 -> use measured zmin
        zmin *= zscale;

        const int width_in_pixels = int(gnuplot_width*600.0/1024);
        const int height_in_pixels_3d = int(gnuplot_width*1200.0/1024);
        int fontsize = std::max(long(12), lrint(12.0*gnuplot_width/1024.0));
        int title_fontsize = std::max(long(14), lrint(14.0*gnuplot_width/1024.0));
        int fontsize3d = std::max(long(9), lrint(9.0*gnuplot_width/1024.0));
        double linewidth = std::max(double(0.5), double(0.5)*gnuplot_width/1024.0);
        
        FILE* gpf = fopen((wdir + std::string("grid.gnuplot")).c_str(), "wt");
        //if (img_filename.length() > 0) {
        //    fprintf(gpf, "set label 11 center at graph 0.5,char 1 \"%s\" font \"Arial,%d\"\n", img_name_clean.c_str(), title_fontsize);
        //    fprintf(gpf, "set bmargin 5\n");
        //}
        
        fprintf(gpf, "%s\n", diverging_palette.c_str());
        fprintf(gpf, "set cbrange [%lf:%lf]\n", zmin, zmax);
        fprintf(gpf, "set xlab \"column (%s)\"\n", lpmm_mode ? "mm" : "pixels");
        fprintf(gpf, "set ylab \"row (%s)\"\n",  lpmm_mode ? "mm" : "pixels");
        fprintf(gpf, "set term png size %d, %d font 'Arial,%d'\n", 
            width_in_pixels, 
            (int)lrint(width_in_pixels*2*grid_mer_fine.rows/double(grid_mer_fine.cols)), 
            fontsize
        );
        fprintf(gpf, "set output \"%sgrid_image.png\"\n", wdir.c_str());
        if (img_filename.length() > 0) {
            fprintf(gpf, "set multiplot title \"%s\" font \"Arial,%d\"\n", img_name_clean.c_str(), title_fontsize);
            fprintf(gpf, "set tmargin 4\n");
        } else {
            fprintf(gpf, "set multiplot\n");
        }
        fprintf(gpf, "set size 1,0.5\n");
        fprintf(gpf, "set origin 0.0,0.5\n");
        fprintf(gpf, "set title \"Meridional\"\n");
        fprintf(gpf, "set yrange [%lf:0] reverse\n", (img.rows-1)/pixel_size);
        fprintf(gpf, "plot [0:%lf] \"%s\" i 2 t \"MTF50 (%s)\" w image\n", 
            (img.cols-1)/pixel_size,
            (wdir+fname).c_str(),
            lpmm_mode ? "lp/mm" : "c/p"
        );
        fprintf(gpf, "set origin 0.0,0.0\n");
        fprintf(gpf, "set title \"Sagittal\"\n");
        fprintf(gpf, "set yrange [%lf:0] reverse\n", (img.rows-1)/pixel_size);
        fprintf(gpf, "plot [0:%lf] \"%s\" i 3 t \"MTF50 (%s)\" w image\n", 
            (img.cols-1)/pixel_size,
            (wdir+fname).c_str(),
            lpmm_mode ? "lp/mm" : "c/p"
        );
        fprintf(gpf, "unset multiplot\n");
        fprintf(gpf, "unset label 11\n");
        fprintf(gpf, "set pm3d hidden3d 1 corners2color median\n");
        fprintf(gpf, "set style line 1 lc rgb \"black\" lw 0.75\n");
        fprintf(gpf, "set term png size %d, %d font \"arial,%d\"\n", 
            (int)lrint(width_in_pixels*2*grid_mer_fine.rows/double(grid_mer_fine.cols)), 
            height_in_pixels_3d,
            fontsize3d
        );
        fprintf(gpf, "set output \"%sgrid_surface.png\"\n", wdir.c_str());
        fprintf(gpf, "unset xlab\n");
        fprintf(gpf, "unset ylab\n");
        if (img_filename.length() > 0) {
            fprintf(gpf, "set multiplot title \"%s\" font \"Arial,%d\"\n", img_name_clean.c_str(), title_fontsize);
            fprintf(gpf, "set tmargin 5\n");
        } else {
            fprintf(gpf, "set multiplot\n");
        }
        fprintf(gpf, "set ticslevel %lf\n", 0.0);
        fprintf(gpf, "set view 25, 350\n");
        fprintf(gpf, "set title \"Meridional\"\n");
        fprintf(gpf, "set size 1,0.5\n");   
        fprintf(gpf, "set origin 0.0,0.5\n");
        fprintf(gpf, "splot [][][%lf:%lf] \"%s\" i 0 w pm3d lc rgb \"black\" lw %lf notitle\n", 
                zmin, zmax,
                (wdir+fname).c_str(),
                linewidth
        );
        fprintf(gpf, "set view 25, 350\n");
        fprintf(gpf, "set title \"Sagittal\"\n");
        fprintf(gpf, "set origin 0.0,0.0\n");
        fprintf(gpf, "splot [][][%lf:%lf] \"%s\" i 1 w pm3d lc rgb \"black\" lw %lf notitle\n", 
                zmin, zmax,
                (wdir+fname).c_str(),
                linewidth
        );
        fprintf(gpf, "unset multiplot\n");
        
        fclose(gpf);
        
        char* buffer = new char[1024];
        #ifdef _WIN32
        sprintf(buffer, "\"\"%s\" \"%sgrid.gnuplot\"\"", gnuplot_binary.c_str(), wdir.c_str());
        #else
        sprintf(buffer, "\"%s\" \"%sgrid.gnuplot\"", gnuplot_binary.c_str(), wdir.c_str());
        #endif
        int rval = system(buffer);
        if (rval != 0) {
            gnuplot_failure = true;
        } else {
           
        }
        
        delete [] buffer;
        
    }
    
    bool gnuplot_failed(void) {
        return gnuplot_failure;
    }

  private:

    typedef enum {MERIDIONAL, SAGITTAL, NEITHER} Edge_type;

    void extract_mtf_grid(Edge_type target_edge_type, cv::Mat& grid_coarse, cv::Mat& grid_fine, const vector<Block>& blocks, double upper) {
        Point2d centr(0,0);
        for (size_t i=0; i < blocks.size(); i++) {
            centr += blocks[i].get_centroid();
        }
        centr = centr*(1.0/double(blocks.size()));
        
        for (size_t i=0; i < blocks.size(); i++) {
            for (size_t k=0; k < 4; k++) {
                double val = blocks[i].get_mtf50_value(k);
                if (val > 0 && val < 1.0) {
                    Point2d cent = blocks[i].get_edge_centroid(k);

                    Point2d dir = cent - centr;
                    dir = dir * (1.0/norm(dir));

                    Point2d norm = blocks[i].get_normal(k);
                    double delta = dir.x*norm.x + dir.y*norm.y;

                    Edge_type edge_type = NEITHER;
                    if (target_edge_type == MERIDIONAL) {
                        if (fabs(delta) > cos(45.0/180*M_PI)) {
                            edge_type = MERIDIONAL;
                        } else {
                            edge_type = SAGITTAL;
                        }
                    } else {
                        if (fabs(delta) > cos(45.0/180*M_PI)) {
                            edge_type = MERIDIONAL;
                        } else {
                            edge_type = SAGITTAL;
                        }
                    }
                    
                    int ly = lrint(cent.y*(grid_y_fine-1)/img.rows); 
                    int lx = lrint(cent.x*(grid_x_fine-1)/img.cols);

                    if (val > grid_fine.at<float>(ly,lx) && edge_type == target_edge_type) { // max composite
                        grid_fine.at<float>(ly,lx) = val;
                    }
                }
                
            }
        }
        
        vector< Eigen::VectorXd > solutions(grid_coarse.rows * grid_coarse.cols, Eigen::VectorXd::Zero(1));
        
        double cell_r = grid_fine.rows / double(grid_coarse.rows);
        double cell_c = grid_fine.cols / double(grid_coarse.cols);
        
        vector<Mtf_profile_sample> samples;
        for (int lr=0; lr < grid_fine.rows; lr++) {
            for (int lc=0; lc < grid_fine.cols; lc++) {
                if (grid_fine.at<float>(lr, lc) > 1e-6) {
                    samples.push_back(Mtf_profile_sample(Point2d(lc, lr), grid_fine.at<float>(lr, lc), 0, 1));
                }
            }
        }
        
        // perform local LS fitting of low-order polynomial
        MatrixXd design(samples.size(), 6);
        VectorXd vy(samples.size());
        for (int t_row=0; t_row < grid_coarse.rows; t_row++) {
            for (int t_col=0; t_col < grid_coarse.cols; t_col++) {
                
                int fine_row = t_row * grid_y_fine / grid_y_coarse;
                int fine_col = t_col * grid_x_fine / grid_x_coarse;
                
                // a.x^2 + b.y^2 + c.x.y + d.x + e.y + f = 0
                double lmax = 0;
                double cover_sum = 0;
                for (size_t i=0; i < samples.size(); i++) {
                    double dist_r = fabs(fine_col - samples[i].p.x)/cell_c;
                    double dist_c = fabs(fine_row - samples[i].p.y)/cell_r;
                    
                    double scale = 11; 
                    
                    cover_sum += exp( -( (SQR(dist_r) + SQR(dist_c)) / scale) ) * ( (samples[i].mtf > 1e-6) ? 1 : 0 ) / (sqrt(2*scale*M_PI));
                    
                    double clip=1.0;
                    
                    // force a linear fit near the edges
                    if (t_row <= 2 || t_col <= 2 || t_row >= grid_coarse.rows - 3 || t_col >= grid_coarse.cols - 3) {
                        clip = 0.0;
                    }
                    
                    
                    double w = exp( -( (SQR(dist_r) + SQR(dist_c)) / scale) );
                    
                    design(i, 0) = 1*w;
                    design(i, 1) = samples[i].p.y*w;
                    design(i, 2) = samples[i].p.x*w;
                    design(i, 3) = clip*samples[i].p.x * samples[i].p.y * w;
                    design(i, 4) = clip*SQR(samples[i].p.y) * w;
                    design(i, 5) = clip*SQR(samples[i].p.x) * w;
                    vy[i] = samples[i].mtf * w;
                    lmax = max(lmax, samples[i].mtf);
                }
                
                lmax = min(lmax, upper);
                
                VectorXd sol = design.colPivHouseholderQr().solve(vy);
                solutions[t_row*grid_coarse.cols + t_col] = sol;
                
                double pred = sol[0] + sol[1]*fine_row + sol[2]*fine_col +
                    sol[3]*fine_row*fine_col + sol[4]*fine_row*fine_row + sol[5]*fine_col*fine_col;
                
                
                double thresh = 0.5;
                if (t_row <= 2 || t_col <= 2 || t_row >= grid_coarse.rows - 3 || t_col >= grid_coarse.cols - 3) {
                    thresh = 0.01;
                }
                
                if (cover_sum < thresh) {
                    pred = 0;
                    solutions[t_row*grid_coarse.cols + t_col].setZero();
                }
                    
                grid_coarse.at<float>(t_row, t_col) = max(0.0, min(pred, lmax));
            }
        }
        
        int width = grid_coarse.cols;
        // use bilinear interpolation to produce a smoother output image
        for (int row=0; row < grid_fine.rows; row++) {
            for (int col=0; col < grid_fine.cols; col++) {
                double y = row*grid_coarse.rows / double(grid_fine.rows);
                double x = col*grid_coarse.cols / double(grid_fine.cols);
                
                int fx = (int)floor(x);
                int fy = (int)floor(y);

                int cx = (int)ceil(x);
                int cy = (int)ceil(y);

                int rx = (int)floor(x+0.5); // x, y positive, so this produces rounding
                int ry = (int)floor(y+0.5);
                
                cx = min(cx, grid_coarse.cols - 1);
                cy = min(cy, grid_coarse.rows - 1);
                rx = min(rx, grid_coarse.cols - 1);
                ry = min(ry, grid_coarse.rows - 1);

                double xfac = x - fx;
                double yfac = y - fy;

                double w1 = (1 - xfac) * (1 - yfac);
                double w2 =      xfac  * (1 - yfac);
                double w3 = (1 - xfac) *      yfac;
                double w4 =      xfac  *      yfac;
                
                
                double val =
                    w1 * predict(solutions, fy, fx, width, row, col) + w2 * predict(solutions, fy, cx, width, row, col) +
                    w3 * predict(solutions, cy, fx, width, row, col) + w4 * predict(solutions, cy, cx, width, row, col);
                
                grid_fine.at<float>(row, col) = max(0.0, min(val, upper)); 
            }
        }
    }
    
    inline double predict(const  vector< Eigen::VectorXd >& solutions, int irow, int icol, int width, double row, double col) const {
        const VectorXd& sol = solutions[irow*width + icol];
        double pred = sol[0] + sol[1]*row + sol[2]*col +
            sol[3]*row*col + sol[4]*row*row + sol[5]*col*col;
        
        return pred;
    }

    static double angular_diff(double a, double b) {
        return acos(cos(a)*cos(b) + sin(a)*sin(b));
    }
    
    std::string wdir;
    std::string fname;
    std::string gnuplot_binary;
    
    // grid used to fit local polynomial
    size_t grid_x_coarse;
    size_t grid_y_coarse;
    
    // grid used to interpolate polynomial at fine scale for image output
    size_t grid_x_fine;
    size_t grid_y_fine;
    
    double img_y;
    double img_x;
    const cv::Mat& img;

    bool    lpmm_mode;
    double  pixel_size;
    
    bool gnuplot_failure;
    bool gnuplot_warning;

    double m_lower;
    double m_upper;
    
    int gnuplot_width;
    double zscale = 0.0;
};

#endif
