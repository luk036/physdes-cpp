#include <recti/halton_int.hpp>

#include "doctest/doctest.h"

TEST_CASE("VDC") {
    CHECK_EQ(recti::vdc(1, 2, 3), 4);
    CHECK_EQ(recti::vdc(2, 2, 3), 2);
    CHECK_EQ(recti::vdc(3, 2, 3), 6);
    CHECK_EQ(recti::vdc(4, 2, 3), 1);
    CHECK_EQ(recti::vdc(5, 2, 3), 5);
    CHECK_EQ(recti::vdc(6, 2, 3), 3);
    CHECK_EQ(recti::vdc(7, 2, 3), 7);
}

TEST_CASE("Vdcorput") {
    auto vdcgen = recti::Vdcorput(2, 3);
    CHECK_EQ(vdcgen(), 4);
    CHECK_EQ(vdcgen(), 2);
    CHECK_EQ(vdcgen(), 6);
    CHECK_EQ(vdcgen(), 1);
    CHECK_EQ(vdcgen(), 5);
    CHECK_EQ(vdcgen(), 3);
    CHECK_EQ(vdcgen(), 7);
}

TEST_CASE("Halton") {
    unsigned base[] = {2, 3};
    unsigned scale[] = {3, 2};
    auto hgen = recti::halton(base, scale);
    CHECK_EQ(hgen(), std::vector<unsigned>{4, 3});
    CHECK_EQ(hgen(), std::vector<unsigned>{2, 6});
    CHECK_EQ(hgen(), std::vector<unsigned>{6, 1});
    CHECK_EQ(hgen(), std::vector<unsigned>{1, 4});
    CHECK_EQ(hgen(), std::vector<unsigned>{5, 7});
    CHECK_EQ(hgen(), std::vector<unsigned>{3, 2});
    CHECK_EQ(hgen(), std::vector<unsigned>{7, 5});
}
