#ifndef SFR_ENTRY_H
#define SFR_ENTRY_H

#include <cmath>
#include <vector>

using std::vector;

class Sfr_entry {
public:

    Sfr_entry(void) : px(-1), py(-1), pz(0), sfr(0), area(0), t_sfr(0), r_sfr(0), b_sfr(0), l_sfr(0), layer(0), location(0) {
    }

    Sfr_entry(const double px, const double py, const double pz,
              const double sfr, const double area, const double t_sfr,
              const double r_sfr, const double b_sfr, const double l_sfr, const int l, const int loc) :
              px(px), py(py), pz(pz), sfr(sfr), area(area),
              t_sfr(t_sfr), r_sfr(r_sfr), b_sfr(b_sfr), l_sfr(l_sfr),
              layer(l), location(loc)
    {}

    double distance(const double nx, const double ny) {
        return sqrt((px - nx)*(px - nx) + (py - ny)*(py - ny));
    }

    void clear(void) {

    }

    double px;
    double py;
    double pz;
    double sfr;
    double area;
    double t_sfr;
    double r_sfr;
    double b_sfr;
    double l_sfr;
    int layer;    // 0 = Center Only ( From Inner layer to outer layer)
    int location; // 0 = Upper Left, 1 = Upper Right, 2 = Lower Right, 3 = Lower Lefr
};

// least-squares fit (inverse of design matrix) of a cubic polynomial through 4 points [-1..2]/64.0
const double sfr_cubic_weights[4][4] = {
    { 0.00000000000000e+00,   1.00000000000000e+00,   0.00000000000000e+00,   0.00000000000000e+00 },
    { -2.13333333333333e+01,  -3.20000000000000e+01,   6.40000000000000e+01,  -1.06666666666667e+01 },
    { 2.04800000000000e+03,  -4.09600000000000e+03,   2.04800000000000e+03,   0.00000000000000e+00 },
    { -4.36906666666667e+04,   1.31072000000000e+05,  -1.31072000000000e+05,   4.36906666666667e+04 }
};


#endif // SFR_ENTRY_H
