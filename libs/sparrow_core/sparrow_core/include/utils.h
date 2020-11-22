#ifndef UTILS_H
#define UTILS_H

#include <Eigen/Eigenvalues>
#include <Eigen/Dense>
#include <Sfr_entry.h>
#include <sparrowcore_global.h>

struct threeDPoint {
    threeDPoint() : x(0), y(0), z(0)
    {

    };
    threeDPoint(double xx, double yy, double zz)
        : x(xx), y(yy), z(zz)
    {
    };
    double x;
    double y;
    double z;
};

void sfrCurveFittingWithOrder(vector<Sfr_entry> v, vector<double> &a, double& r2, double& x_min, double& x_max, double &ex, double &ey, double &ez, int order = 6);
void sfrCurveFitting(vector<Sfr_entry> v, vector<double> &a, double& r2, double& x_min, double& x_max, double &ex, double &ey, double &ez);
void curveFitting(double x[], double y[], const int sampleSize, double* a, double& r2);
SPARROWCORESHARED_EXPORT threeDPoint planeFitting(std::vector<threeDPoint> points);
SPARROWCORESHARED_EXPORT vector<vector<Sfr_entry>> sfr_curve_analysis(vector<vector<Sfr_entry>> const sfr_sampling);

#endif // UTILS_H
