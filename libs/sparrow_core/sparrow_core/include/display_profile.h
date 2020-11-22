#ifndef DISPLAY_PROFILE_H
#define DISPLAY_PROFILE_H

#include <stdio.h>
#include <string>
using std::string;

#include <vector>
using std::vector;
using std::pair;

#include <array>

#include <opencv2/core/core.hpp>

class Display_profile {
  public:
    Display_profile(void);
    Display_profile(const vector<double>& gparm, const vector<double>& luminance_weights);
    Display_profile(const vector< pair<uint16_t, uint16_t> >& gtable, const vector<double>& luminance_weights);

    void force_linear(void) { is_linear = true; }
    void force_sRGB(void);
    cv::Mat to_luminance(const cv::Mat& img);

  private:
    void render_parametric(const vector<double>& gparm);

    std::array<uint16_t, 65536> lut;
    vector<double> luminance_weights;
    bool is_linear = false;

};

#endif
