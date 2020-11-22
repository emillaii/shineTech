#ifndef VISIONAVADAPTOR_H
#define VISIONAVADAPTOR_H

#include <vector>
#include <tuple>
#include <opencv2/opencv.hpp>
#include "sfr_entry.h"
#include <QPoint>
#include <QImage>
#include <sparrowcore_global.h>

namespace AA_Helper {

    void SPARROWCORESHARED_EXPORT AA_Find_Charactertistics_Pattern(std::vector<std::vector<Sfr_entry>> clustered_sfr_v, int imageWidth, int imageHeight,
                                          unsigned int & ccROIIndex,
                                          unsigned int & ulROIIndex,
                                          unsigned int & urROIIndex,
                                          unsigned int & llROIIndex,
                                          unsigned int & lrROIIndex);

    struct SPARROWCORESHARED_EXPORT patternAttr
    {
        QPointF center;
        double width;
        double height;
        double area;
    };
    std::vector<patternAttr> SPARROWCORESHARED_EXPORT AA_Search_MTF_Pattern(cv::Mat inImage, QImage & image, bool isFastMode,
                                                                            unsigned int & ccROIIndex,
                                                                            unsigned int & ulROIIndex,
                                                                            unsigned int & urROIIndex,
                                                                            unsigned int & llROIIndex,
                                                                            unsigned int & lrROIIndex,
                                                                            int max_intensity, int min_area, int max_area
                                                                            );
    std::vector<patternAttr> SPARROWCORESHARED_EXPORT AAA_Search_MTF_Pattern_Ex(cv::Mat inImage,int max_intensity, int min_area, int max_area, int layer);
    double SPARROWCORESHARED_EXPORT calculateAACornerDeviation(double ul_z, double ur_z, double ll_z, double lr_z);

    bool SPARROWCORESHARED_EXPORT calculateOC(cv::Mat image, QPointF &center, QImage & outImage, int intensity_threshold = 100);

    bool SPARROWCORESHARED_EXPORT calculateImageIntensityProfile(cv::Mat image, float &min_intensity, float & max_intensity, vector<float> &intensityProfile, int mode, int margin, int &numberOfDetectedError, float &negative_change_intensity, float &positive_change_intensity);
    bool SPARROWCORESHARED_EXPORT calculateImageIntensityProfileWithCustomPath(QString pathFilename, QString saveFilename, cv::Mat image, float &min_intensity, float & max_intensity, vector<float> &intensityProfile, int &numberOfDetectedError, float &negative_change_intensity, float &positive_change_intensity);
}

#endif // VISIONAVADAPTOR_H
