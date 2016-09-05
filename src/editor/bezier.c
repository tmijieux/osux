#include <glib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

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

bezier_point Bezier(uint32_t n, double t, bezier_point W[restrict (n+1)])
{
    if (!bz_inited)
        init_binomials(MAX_BINOMIAL);
            
    bezier_point p = { 0.0, 0.0 };
    for (uint32_t i = 0; i <= n; ++i) {
        double v = bezier_binomial(n, i) * pow(1-t, n-i) * pow(t, i);
        p.x += v * W[i].x;
        p.y += v * W[i].y;
    }
    return p;
}
