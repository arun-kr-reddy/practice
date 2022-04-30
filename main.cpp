#include <iostream>

#include "blur.hpp"
#include "edge.hpp"
#include "histogram.hpp"
#include "pgm.hpp"
#include "resize.hpp"

int main(void) {

    pgm_t src_pgm("../images/lenna.pgm");
    src_pgm.write("./1.pgm");
    histogram(src_pgm);
    src_pgm.write("./2.pgm");

    pgm_t blur_pgm(src_pgm.width(), src_pgm.height());

    blur(src_pgm, blur_pgm, 3, clamp);
    blur_pgm.write("./3.pgm");

    pgm_t edgeX_pgm(src_pgm.width(), src_pgm.height());
    edgeX(blur_pgm, edgeX_pgm, clamp, 0);
    edgeX_pgm.write("./4.pgm");

    pgm_t edgeY_pgm(src_pgm.width(), src_pgm.height());
    edgeY(blur_pgm, edgeY_pgm, clamp, 0);
    edgeY_pgm.write("./5.pgm");

    pgm_t dst_pgm(src_pgm.width(), src_pgm.height());
    edgeRms(edgeX_pgm, edgeY_pgm, dst_pgm, 130);
    dst_pgm.write("./6.pgm");

    pgm_t rsz_pgm(1024, 1024);
    resize(src_pgm, rsz_pgm, bilinear);
    rsz_pgm.write("./7.pgm");

    return 0;
}
