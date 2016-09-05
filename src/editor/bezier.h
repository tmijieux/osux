#ifndef BEZIER_H
#define BEZIER_H

#define MAX_BINOMIAL 30

typedef struct bezier_point_ {
    double x, y;
} bezier_point;

bezier_point Bezier_de_Casteljau(uint32_t n, // control point count = n+1
                                 double t, // t € [ 0, 1 ]
                                 bezier_point W[n+1]); // control points

#endif //BEZIER_H
