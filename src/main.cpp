#include <iostream>

#include "blur.hpp"
#include "edge.hpp"
#include "histogram.hpp"
#include "ocl.hpp"
#include "pgm.hpp"
#include "resize.hpp"

int main(int argc, char const *argv[])
{
#if 1
    assert(argc == 2);
    std::string input_filename = std::string(argv[1]);

    pgm_t src_pgm(input_filename);
    src_pgm.write("./1_input.pgm");
    histogram(src_pgm);
    src_pgm.write("./2_histogram_equalized.pgm");

    pgm_t blur_pgm(src_pgm.width(), src_pgm.height());

    blur(src_pgm, blur_pgm, clamp);
    blur_pgm.write("./3_blured.pgm");

    pgm_t edgeX_pgm(src_pgm.width(), src_pgm.height());
    edgeX(src_pgm, edgeX_pgm, clamp);
    edgeX_pgm.write("./4_edgeX.pgm");

    pgm_t edgeY_pgm(src_pgm.width(), src_pgm.height());
    edgeY(src_pgm, edgeY_pgm, clamp);
    edgeY_pgm.write("./5_edgeY.pgm");

    pgm_t dst_pgm(src_pgm.width(), src_pgm.height());
    edgeRms(edgeX_pgm, edgeY_pgm, dst_pgm, 0);
    dst_pgm.write("./6_edgeRMS.pgm");

    pgm_t rsz_pgm(1024, 1024);
    resize(src_pgm, rsz_pgm, bilinear);
    rsz_pgm.write("./7_resize.pgm");
#else
    matrix_multiplication(); //FIXME: not working
#endif
    return 0;
}
