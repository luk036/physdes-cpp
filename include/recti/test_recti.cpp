#include "recti.hpp"

using namespace recti;

template <typename Obj>
auto test_recti(Obj& obj)
{
    auto obj2 = obj;
    assert(obj2 == obj);
    auto obj3 = obj2.flip().flip();
    assert(obj3 == obj);
}


int main()
{
    auto v = vector2(1, 2);
    auto p = point(3, 4);
    auto q = point(5, 6);
    auto intv1 = interval(2, 8);
    auto intv3 = interval(1, 10);
    auto R = rectangle(intv1, intv3);
    auto vseg = vsegment(4, intv1);
    auto hseg = hsegment(intv3, 11);

    auto v2 = v;
    assert(v2 == v);

    auto intv2 = intv1;
    assert(intv2 == intv1);
    assert(intv2.contains(5));
    assert(intv3.contains(intv2));

    test_recti(p);
    test_recti(vseg);
    test_recti(hseg);
    test_recti(R);

    assert(R.contains(p));
    assert(R.contains(vseg));
    assert(!R.contains(hseg));
}
