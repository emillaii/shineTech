#ifndef MTF_MAPPER_H
#define MTF_MAPPER_H

#include "include/common_types.h"
#include "include/gamma_lut.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

void convert_8bit_input(cv::Mat& cvimg, bool gamma_correct=true);

#endif // MTF_MAPPER_H
