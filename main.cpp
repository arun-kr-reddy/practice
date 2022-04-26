#include <iostream>

#include "blur.hpp"
#include "edge.hpp"
#include "histogram.hpp"
#include "pgm.hpp"

int main(void) {

    pgm src_pgm("../images/lenna.pgm");
    src_pgm.write("./1.pgm");
    histogram(src_pgm);
    src_pgm.write("./2.pgm");

    pgm blur_pgm = src_pgm;

    blur(src_pgm, blur_pgm, 5);
    blur_pgm.write("./3.pgm");

    pgm edgeX_pgm = blur_pgm;
    edgeX(blur_pgm, edgeX_pgm, 132);
    edgeX_pgm.write("./4.pgm");

    pgm dst_pgm = blur_pgm;
    edgeY(edgeX_pgm, dst_pgm, 132);
    dst_pgm.write("./5.pgm");

#if 0
    apply_edgeX(&hist_pgm, &edgeX_pgm, 132);
    pgm::writePGM("./3.pgm", &edgeX_pgm);

    apply_edgeY(&edgeX_pgm, &dst_pgm, 132);
    pgm::writePGM("./4.pgm", &dst_pgm);
#endif
    return 0;
}
