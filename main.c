#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blur.h"
#include "edge.h"
#include "histogram.h"
#include "pgm.h"

int main(void) {
    pgm_t src_pgm;
    pgm_t hist_pgm;
    pgm_t edgeX_pgm;
    pgm_t dst_pgm;
    histogram_t hist;

    readPGM("../images/lenna.pgm", &src_pgm);
    printf("%d x %d\n", src_pgm.width, src_pgm.height);
    writePGM("./1.pgm", &src_pgm);

    dst_pgm.height = src_pgm.height;
    dst_pgm.width = src_pgm.width;
    dst_pgm.max_gray = src_pgm.max_gray;
    dst_pgm.ptr = malloc(dst_pgm.height * dst_pgm.width);

    hist_pgm.height = src_pgm.height;
    hist_pgm.width = src_pgm.width;
    hist_pgm.max_gray = src_pgm.max_gray;
    hist_pgm.ptr = malloc(hist_pgm.height * hist_pgm.width);

    edgeX_pgm.height = src_pgm.height;
    edgeX_pgm.width = src_pgm.width;
    edgeX_pgm.max_gray = src_pgm.max_gray;
    edgeX_pgm.ptr = malloc(edgeX_pgm.height * edgeX_pgm.width);

    calc_histogram(&src_pgm, &hist);
    calc_cum_freq(&hist);
    memcpy(hist_pgm.ptr, src_pgm.ptr, hist_pgm.width * hist_pgm.height);
    apply_hist_eq(&hist_pgm, &hist);
    writePGM("./2.pgm", &hist_pgm);

    apply_edgeX(&hist_pgm, &edgeX_pgm, 132);
    writePGM("./3.pgm", &edgeX_pgm);

    apply_edgeY(&edgeX_pgm, &dst_pgm, 132);
    writePGM("./4.pgm", &dst_pgm);

    return 0;
}
