#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H

#include <vector>
#include <opencv2/core/core.hpp>
#include <sparrowcore_global.h>

namespace image_util {

SPARROWCORESHARED_EXPORT std::vector<std::tuple<int, int, cv::Mat> > searchROI(const cv::Mat &inImage,
                                                      unsigned int &centerROIIndex,
                                                      unsigned int & ulROIIndex,
                                                      unsigned int & urROIIndex,
                                                      unsigned int & llROIIndex,
                                                      unsigned int & lrROIIndex);
/*
inline int ROICenterScore(int rows, int columns, int startRow, int endRow, int startColumn, int endColumn);
inline int ROIUpperLeftScore(int rows, int columns, int startRow, int endRow, int startColumn, int endColumn);
inline int ROIUpperRightScore(int rows, int columns, int startRow, int endRow, int startColumn, int endColumn);
inline int ROILowerLeftScore(int rows, int columns, int startRow, int endRow, int startColumn, int endColumn);
inline int ROILowerRightScore(int rows, int columns, int startRow, int endRow, int startColumn, int endColumn);
*/

}

#endif // IMAGE_UTIL_H
