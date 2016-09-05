#include <glib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "osux/util.h"
#include "osux/error.h"
#include "bezier.h"

static bool bz_inited = false;
static int64_t **binomials = NULL;
static unsigned bz_size;

static void deinit_binomials(void)
{
    for (unsigned i = 0; i < bz_size; ++i) {
        g_free(binomials[i]);
    }
    g_free(binomials);
    binomials = NULL;
    bz_size = 0;
    bz_inited = false;
}

static void init_binomials(int n)
{
    deinit_binomials();
    bz_size = n;
    binomials = g_malloc(sizeof*binomials * n);
    for (int i = 0; i < n; ++i) {
        binomials[i] = g_malloc0(sizeof *binomials[i] * (i+1));
        binomials[i][0] = 1;
        binomials[i][i] = 1;
        for (int j = 1; j < i-1; ++j)
            binomials[i][j] = binomials[i-1][j-1] + binomials[i-1][j];
    }
    bz_inited = true;
}

int64_t bezier_binomial(uint32_t n, uint32_t k)
{
    if (n >= bz_size) {
        osux_error("binomial size is too big: %u\n", n);
        return -1;
    }
    return binomials[n][k];
}

bezier_point Bezier_de_Casteljau(uint32_t n, // control point count = n+1
                                 double t, // t € [ 0, 1 ]
                                 bezier_point W[n+1]) // control points
{
    int s = n+1;
    bezier_point *X = ARRAY_DUP(W, s);
    bezier_point *Y = g_malloc(sizeof*Y * s);
    bezier_point *tmp, p;

    while (s > 1) {
        for (int i = 0; i < s-1; ++i) {
            Y[i].x = (1-t) * X[i].x + t * X[i+1].x;
            Y[i].y = (1-t) * X[i].y + t * X[i+1].y;
        }
        SWAP_POINTER(X, Y, tmp);
        -- s;
    }
    p = X[0];
    g_free(X);
    g_free(Y);
    return p;
}
