#ifndef AA_UTIL_H
#define AA_UTIL_H

#include <sparrowcore_global.h>
#include <sfr_entry.h>
#include <utils.h>


#include <vector>

namespace aa_util {

struct SPARROWCORESHARED_EXPORT aaCurve {
    aaCurve() : z_min(0), z_max(0)
    {
    };

    aaCurve(double zmin, double zmax, std::vector<double> && cv)
         : z_min(zmin)
         , z_max(zmax)
         , c(cv)
    {
    };

    std::vector<double> c;
    double z_min;
    double z_max;
};


SPARROWCORESHARED_EXPORT void sfrFitAllCurves(const std::vector<std::vector<Sfr_entry> > &clustered_sfr_v, std::vector<aaCurve> &aaCurves, std::vector<threeDPoint> & points, double & g_x_min, double & g_x_max, double &cc_peak_z, int & cc_curve_index, const int & principle_center_x = 0, const int & principle_center_y = 0, double sensor_x_scale = 500, double sensor_y_scale = 500);
}

#endif // AA_UTIL_H
