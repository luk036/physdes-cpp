#include <doctest/doctest.h>

#include <recti/halton_int.hpp>

TEST_CASE("VDC 2") {
    CHECK_EQ(recti::vdc(1, 3, 7), 729);
    CHECK_EQ(recti::vdc(2, 3, 7), 1458);
    CHECK_EQ(recti::vdc(3, 3, 7), 243);
}

TEST_CASE("Vdcorput 2") {
    auto vdcgen = recti::Vdcorput(3, 7);
    CHECK_EQ(vdcgen(), 729);
    CHECK_EQ(vdcgen(), 1458);
    CHECK_EQ(vdcgen(), 243);
}

TEST_CASE("Halton 2") {
    unsigned base[] = {3, 5};
    unsigned scale[] = {7, 7};
    auto hgen = recti::halton(base, scale);
    CHECK_EQ(hgen(), std::vector<unsigned>{729, 15625});
    CHECK_EQ(hgen(), std::vector<unsigned>{1458, 31250});
    CHECK_EQ(hgen(), std::vector<unsigned>{243, 46875});
}