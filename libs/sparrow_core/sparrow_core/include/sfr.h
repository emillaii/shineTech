#ifndef SFR_H
#define SFR_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "sfr_entry.h"
#include <image_util.h>
#include <sparrowcore_global.h>
struct MTF_Pattern_Position {
    double x = 0;
    double y = 0;
    double radius = 0;
    int layer = 0;
    double field = 0;
    double t_sfr = 0;
    double r_sfr = 0;
    double b_sfr = 0;
    double l_sfr = 0;
    double area = 0;
    double avg_sfr = 0;
    MTF_Pattern_Position(double xx, double yy, double rr)
        : x(xx), y(yy), radius(rr){
    }
    MTF_Pattern_Position(double xx, double yy, double rr, double tsfr, double rsfr, double bsfr, double lsfr, double a, double avgsfr)
        : x(xx), y(yy), radius(rr), t_sfr(tsfr), r_sfr(rsfr), b_sfr(bsfr), l_sfr(lsfr), area(a), avg_sfr(avgsfr){
    }
};

class PositionRadianComp {
    double cx;
    double cy;
public:
    PositionRadianComp(int centerX, int centerY) : cx(centerX), cy(centerY) {}

    bool operator()(const MTF_Pattern_Position & p1, const MTF_Pattern_Position & p2) {
        // logic uses param
        if (p1.x < cx && p2.x > cx)
            return true;
        if (p1.x > cx && p2.x < cx) {
            return false;
        }
        double crossProduct = (p1.x - cx) * (p2.y - cy) - (p1.y - cy) * (p2.x - cx);
        return crossProduct < 0;
    }
};
class SPARROWCORESHARED_EXPORT sfr
{
public:
    enum EdgeFilter {
        NO_FILTER,
        VERTICAL_ONLY,
        HORIZONTAL_ONLY
    };

    typedef std::vector<std::tuple<int, int, cv::Mat> > searchROI_func_type(const cv::Mat &inImage,
                                                                  unsigned int &centerROIIndex,
                                                                  unsigned int & ulROIIndex,
                                                                  unsigned int & urROIIndex,
                                                                  unsigned int & llROIIndex,
                                                                  unsigned int & lrROIIndex);
    static void sfr_calculation(std::vector<std::tuple<double, double, vector<double>>> &v_sfr, cv::Mat& cvimg, int freq_factor = 1,  EdgeFilter filter = NO_FILTER);
    static vector<Sfr_entry> calculateSfr(double currZPos, cv::Mat& cvimg, int freq_factor = 1, EdgeFilter filter = NO_FILTER);
    static vector<Sfr_entry> calculateSfrWithRoiBreakdown(double currZPos, cv::Mat& cvimg, bool isDebug, searchROI_func_type searchROI = image_util::searchROI);
    static vector<Sfr_entry> calculateDFOVEx(cv::Mat& cvimg);
    static double calculateSfrWithSingleRoi(cv::Mat& cvimg, int freq_factor = 1);
    static bool PositionComp(const MTF_Pattern_Position & p1, const MTF_Pattern_Position & p2);
    static vector<int> classifyLayers(std::vector<MTF_Pattern_Position> & vec, int x = 0, int y = 0, double threshold = 0.1);
private:
    sfr() {}
};


#endif // SFR_H
