#ifndef BEZIER_H
#define BEZIER_H

#define MAX_BINOMIAL 30

typedef struct bezier_point_ {
    double x, y;
} bezier_point;

bezier_point Bezier(uint32_t n, double t, bezier_point W[restrict (n+1)]);


#endif //BEZIER_H
