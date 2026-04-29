#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <span>
#include <utility>
#include <vector>

#include <recti/point.hpp>
#include <recti/rdllist.hpp>
#include <recti/rpolygon.hpp>

namespace recti {

    template <typename FwIter, typename KeyFn, typename CmpFn>
    auto create_mono_rpolygon(FwIter&& first, FwIter&& last, const KeyFn& dir, const CmpFn& cmp)
        -> bool {
        assert(first != last);

        auto compare_by_direction = [&dir](const auto& right_elem, const auto& left_elem) -> bool {
            return dir(right_elem) < dir(left_elem);
        };
        auto result = std::minmax_element(first, last, compare_by_direction);
        const auto leftmost = *result.first;
        const auto rightmost = *result.second;

        const auto is_anticw = cmp(dir(leftmost).second, dir(rightmost).second);
        auto is_right_to_left_chain = [&leftmost, &dir](const auto& element) -> bool {
            return dir(element).second <= dir(leftmost).second;
        };
        auto is_left_to_right_chain = [&leftmost, &dir](const auto& element) -> bool {
            return dir(element).second >= dir(leftmost).second;
        };
        const auto middle = is_anticw
                                ? std::partition(first, last, std::move(is_right_to_left_chain))
                                : std::partition(first, last, std::move(is_left_to_right_chain));
        std::sort(first, middle, compare_by_direction);
        std::sort(middle, last, std::move(compare_by_direction));
        std::reverse(middle, last);
        return is_anticw;
    }

    template <typename FwIter>
    auto create_xmono_rpolygon(FwIter&& first, FwIter&& last) -> bool {
        return create_mono_rpolygon(
            first, last,
            [](const auto& point) { return std::make_pair(point.xcoord(), point.ycoord()); },
            [](const auto& elem_a, const auto& elem_b) -> bool { return elem_a < elem_b; });
    }

    template <typename FwIter>
    auto create_ymono_rpolygon(FwIter&& first, FwIter&& last) -> bool {
        return create_mono_rpolygon(
            first, last,
            [](const auto& point) { return std::make_pair(point.ycoord(), point.xcoord()); },
            [](const auto& elem_a, const auto& elem_b) -> bool { return elem_a > elem_b; });
    }

    // Explicit instantiations for std::vector<Point<int>>::iterator
    template auto create_xmono_rpolygon<std::vector<Point<int>>::iterator>(
        std::vector<Point<int>>::iterator&&, std::vector<Point<int>>::iterator&&) -> bool;

    template auto create_ymono_rpolygon<std::vector<Point<int>>::iterator>(
        std::vector<Point<int>>::iterator&&, std::vector<Point<int>>::iterator&&) -> bool;

    template <typename T>
    auto rpolygon_is_xmonotone(std::span<const Point<T>> pointset) -> bool {
        auto x_key = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.xcoord(), pt.ycoord()}; };
        return rpolygon_is_monotone(pointset, x_key);
    }

    template <typename T>
    auto rpolygon_is_ymonotone(std::span<const Point<T>> pointset) -> bool {
        auto y_key = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.ycoord(), pt.xcoord()}; };
        return rpolygon_is_monotone(pointset, y_key);
    }

    template <typename T>
    auto rpolygon_is_convex(std::span<const Point<T>> pointset) -> bool {
        return rpolygon_is_xmonotone(pointset) && rpolygon_is_ymonotone(pointset);
    }

    template <typename T>
    auto point_in_rpolygon(std::span<const Point<T>> pointset, const Point<T>& query_point) -> bool {
        auto previous_point = pointset.back();
        const auto& query_y = query_point.ycoord();
        const auto& previous_y = previous_point.ycoord();

        auto result = false;
        for (const auto& current_point : pointset) {
            const auto& current_y = current_point.ycoord();
            if ((current_y <= query_y && query_y < previous_y)
                || (previous_y <= query_y && query_y < current_y)) {
                if (current_point.xcoord() > query_point.xcoord()) {
                    result = !result;
                }
            }
            previous_point = current_point;
        }
        return result;
    }

    template <typename T>
    auto rpolygon_is_anticlockwise(std::span<const Point<T>> pointset) -> bool {
        const auto min_iterator = std::min_element(pointset.begin(), pointset.end());
        const auto prev_iterator = min_iterator != pointset.begin() ? std::prev(min_iterator)
                                                                    : std::prev(pointset.end());
        return prev_iterator->ycoord() > min_iterator->ycoord();
    }

    // Explicit instantiations for std::span<const Point<int>>
    template auto rpolygon_is_xmonotone<int>(std::span<const Point<int>>) -> bool;
    template auto rpolygon_is_ymonotone<int>(std::span<const Point<int>>) -> bool;
    template auto rpolygon_is_convex<int>(std::span<const Point<int>>) -> bool;
    template auto point_in_rpolygon<int>(std::span<const Point<int>>, const Point<int>&) -> bool;
    template auto rpolygon_is_anticlockwise<int>(std::span<const Point<int>>) -> bool;

}
