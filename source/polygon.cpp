#include <algorithm>
#include <cassert>
#include <span>
#include <utility>
#include <vector>

#include <recti/point.hpp>
#include <recti/polygon.hpp>

namespace recti {

    template <typename FwIter, typename Compare>
    void create_mono_polygon(FwIter&& first, FwIter&& last, const Compare& dir) {
        assert(first != last);

        auto result = std::minmax_element(first, last, dir);
        auto min_pt = *result.first;
        auto max_pt = *result.second;
        auto displace = max_pt - min_pt;
        auto middle = std::partition(first, last, [&displace, &min_pt](const auto& elem) -> bool {
            return displace.cross(elem - min_pt) <= 0;
        });
        std::sort(first, middle, dir);
        std::sort(middle, last, dir);
        std::reverse(middle, last);
    }

    template <typename FwIter>
    auto create_xmono_polygon(FwIter&& first, FwIter&& last) -> void {
        return create_mono_polygon(first, last, [](const auto& lhs, const auto& rhs) -> bool {
            return std::make_pair(lhs.xcoord(), lhs.ycoord())
                   < std::make_pair(rhs.xcoord(), rhs.ycoord());
        });
    }

    template <typename FwIter>
    auto create_ymono_polygon(FwIter&& first, FwIter&& last) -> void {
        return create_mono_polygon(first, last, [](const auto& lhs, const auto& rhs) -> bool {
            return std::make_pair(lhs.ycoord(), lhs.xcoord())
                   < std::make_pair(rhs.ycoord(), rhs.xcoord());
        });
    }

    template <typename T, typename DirFunc>
    auto polygon_is_monotone(std::span<const Point<T>> pointset, const DirFunc& dir) -> bool {
        if (pointset.size() <= 3) {
            return true;
        }

        const size_t n = pointset.size();

        auto [min_it, max_it] = std::minmax_element(
            pointset.begin(), pointset.end(),
            [&dir](const Point<T>& a, const Point<T>& b) { return dir(a) < dir(b); });

        size_t min_index = static_cast<size_t>(std::distance(pointset.begin(), min_it));
        size_t max_index = static_cast<size_t>(std::distance(pointset.begin(), max_it));

        size_t i = min_index;
        while (i != max_index) {
            size_t next_i = (i + 1) % n;
            auto current_key = dir(pointset[i]);
            auto next_key = dir(pointset[next_i]);

            if (current_key.first > next_key.first) {
                return false;
            }
            i = next_i;
        }

        i = max_index;
        while (i != min_index) {
            size_t next_i = (i + 1) % n;
            auto current_key = dir(pointset[i]);
            auto next_key = dir(pointset[next_i]);

            if (current_key.first < next_key.first) {
                return false;
            }
            i = next_i;
        }

        return true;
    }

    template <typename T>
    auto polygon_is_xmonotone(std::span<const Point<T>> pointset) -> bool {
        auto x_key = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.xcoord(), pt.ycoord()}; };
        return polygon_is_monotone(pointset, x_key);
    }

    template <typename T>
    auto polygon_is_ymonotone(std::span<const Point<T>> pointset) -> bool {
        auto y_key = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.ycoord(), pt.xcoord()}; };
        return polygon_is_monotone(pointset, y_key);
    }

    template <typename T>
    auto point_in_polygon(std::span<const Point<T>> pointset, const Point<T>& ptq) -> bool {
        if (pointset.empty()) return false;

        bool res = false;
        auto pt0 = pointset.back();
        const auto qy = ptq.ycoord();

        for (const auto& pt1 : pointset) {
            const auto y0 = pt0.ycoord();
            const auto y1 = pt1.ycoord();

            if ((y1 <= qy && qy < y0) || (y0 <= qy && qy < y1)) {
                const auto det = (ptq - pt0).cross(pt1 - pt0);
                if (y1 > y0) {
                    if (det < 0) {
                        res = !res;
                    }
                } else {
                    if (det > 0) {
                        res = !res;
                    }
                }
            }
            pt0 = pt1;
        }
        return res;
    }

    template <typename T>
    auto polygon_is_anticlockwise(std::span<const Point<T>> pointset) -> bool {
        const auto it1 = std::min_element(pointset.begin(), pointset.end());
        const auto it0 = it1 != pointset.begin() ? std::prev(it1) : std::prev(pointset.end());
        const auto it2 = std::next(it1) != pointset.end() ? std::next(it1) : pointset.begin();
        return (*it1 - *it0).cross(*it2 - *it1) > 0;
    }

    // Explicit instantiations for std::vector<Point<int>>::iterator
    template auto create_xmono_polygon<std::vector<Point<int>>::iterator>(
        std::vector<Point<int>>::iterator&&, std::vector<Point<int>>::iterator&&) -> void;

    template auto create_ymono_polygon<std::vector<Point<int>>::iterator>(
        std::vector<Point<int>>::iterator&&, std::vector<Point<int>>::iterator&&) -> void;

    // Explicit instantiations for std::span<const Point<int>>
    template auto polygon_is_xmonotone<int>(std::span<const Point<int>>) -> bool;
    template auto polygon_is_ymonotone<int>(std::span<const Point<int>>) -> bool;
    template auto point_in_polygon<int>(std::span<const Point<int>>, const Point<int>&) -> bool;
    template auto polygon_is_anticlockwise<int>(std::span<const Point<int>>) -> bool;

}
