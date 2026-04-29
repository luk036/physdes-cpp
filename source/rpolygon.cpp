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
#include <recti/rdllist.hpp>

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


    /**
     * @brief Check if a polygon is monotone with respect to a given direction function
     *
     * A polygon is monotone with respect to a direction if it can be divided into two chains
     * that are both monotone (either entirely non-decreasing or non-increasing) in that direction.
     *
     * @tparam T The type of the coordinates
     * @tparam DirFunc The type of the direction function
     * @param pointset The polygon vertices as points
     * @param dir The direction function that returns a key for comparison
     * @return true if the polygon is monotone, false otherwise
     */
    template <typename T, typename DirFunc>
    inline auto rpolygon_is_monotone(std::span<const Point<T>> pointset, const DirFunc& dir)
        -> bool {
        if (pointset.size() <= 3) {
            return true;
        }

        size_t min_index = 0;
        size_t max_index = 0;
        auto min_val = dir(pointset[0]);
        auto max_val = dir(pointset[0]);

        for (size_t i = 1; i < pointset.size(); ++i) {
            auto current_val = dir(pointset[i]);
            if (current_val < min_val) {
                min_val = current_val;
                min_index = i;
            }
            if (current_val > max_val) {
                max_val = current_val;
                max_index = i;
            }
        }

        RDllist rdll(pointset.size());
        auto& v_min = rdll[min_index];
        auto& v_max = rdll[max_index];

        auto violate
            = [&pointset, &dir](Dllink<size_t>* vertex_iterator, Dllink<size_t>* vertex_stop,
                                std::function<bool(T, T)> cmp) -> bool {
            auto current = vertex_iterator;
            while (current != vertex_stop) {
                auto next_vertex = current->next;
                auto current_key = dir(pointset[current->data]);
                auto next_key = dir(pointset[next_vertex->data]);
                if (cmp(std::get<0>(current_key), std::get<0>(next_key))) {
                    return true;
                }
                current = next_vertex;
            }
            return false;
        };

        if (violate(&v_min, &v_max, [](T value_a, T value_b) { return value_a > value_b; })) {
            return false;
        }

        return !violate(&v_max, &v_min, [](T value_a, T value_b) { return value_a < value_b; });
    }

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

    template <typename FwIter>
    void create_test_rpolygon_old(FwIter&& first, FwIter&& last) {
        assert(first != last);

        auto upwd = [](const auto& right_point, const auto& left_point) -> bool {
            return std::make_pair(right_point.ycoord(), right_point.xcoord())
                   < std::make_pair(left_point.ycoord(), left_point.xcoord());
        };
        auto down = [](const auto& right_point, const auto& left_point) -> bool {
            return std::make_pair(right_point.ycoord(), right_point.xcoord())
                   > std::make_pair(left_point.ycoord(), left_point.xcoord());
        };
        auto left = [](const auto& right_point, const auto& left_point) {
            return std::make_pair(right_point.xcoord(), right_point.ycoord())
                   < std::make_pair(left_point.xcoord(), left_point.ycoord());
        };
        auto right = [](const auto& right_point, const auto& left_point) {
            return std::make_pair(right_point.xcoord(), right_point.ycoord())
                   > std::make_pair(left_point.xcoord(), left_point.ycoord());
        };

        auto result = std::minmax_element(first, last, upwd);
        auto min_pt = *result.first;
        auto max_pt = *result.second;
        auto delta_x = max_pt.xcoord() - min_pt.xcoord();
        auto delta_y = max_pt.ycoord() - min_pt.ycoord();
        auto middle
            = std::partition(first, last, [&min_pt, &delta_x, &delta_y](const auto& elem) -> bool {
                  return delta_x * (elem.ycoord() - min_pt.ycoord())
                         < (elem.xcoord() - min_pt.xcoord()) * delta_y;
              });
        auto max_pt1 = *std::max_element(first, middle, left);
        auto middle2 = std::partition(first, middle, [&max_pt1](const auto& elem) -> bool {
            return elem.ycoord() < max_pt1.ycoord();
        });
        auto min_pt2 = *std::min_element(middle, last, left);
        auto middle3 = std::partition(middle, last, [&min_pt2](const auto& elem) -> bool {
            return elem.ycoord() > min_pt2.ycoord();
        });

        if (delta_x < 0) {
            std::sort(first, middle2, down);
            std::sort(middle2, middle, left);
            std::sort(middle, middle3, upwd);
            std::sort(middle3, last, right);
        } else {
            std::sort(first, middle2, left);
            std::sort(middle2, middle, upwd);
            std::sort(middle, middle3, right);
            std::sort(middle3, last, down);
        }
    }

    template <typename FwIter>
    auto create_test_rpolygon(FwIter first, FwIter last)
        -> std::vector<typename std::iterator_traits<FwIter>::value_type> {
        using T = typename std::iterator_traits<FwIter>::value_type::value_type;
        assert(first != last);

        auto dir_x
            = [](const auto& point) { return std::make_pair(point.xcoord(), point.ycoord()); };
        auto dir_y
            = [](const auto& point) { return std::make_pair(point.ycoord(), point.xcoord()); };

        auto max_point
            = *std::max_element(first, last, [&dir_y](const auto& elem_a, const auto& elem_b) {
                  return dir_y(elem_a) < dir_y(elem_b);
              });
        auto min_point
            = *std::min_element(first, last, [&dir_y](const auto& elem_a, const auto& elem_b) {
                  return dir_y(elem_a) < dir_y(elem_b);
              });
        Vector2<T> vec = max_point - min_point;

        std::vector<typename std::iterator_traits<FwIter>::value_type> upper_chain_points,
            lower_chain_points;
        auto middle = std::partition(first, last, [&min_point, &vec](const auto& point) {
            return vec.cross(point - min_point) < 0;
        });
        upper_chain_points.assign(first, middle);
        lower_chain_points.assign(middle, last);

        auto max_point1 = *std::max_element(upper_chain_points.begin(), upper_chain_points.end(),
                                            [&dir_x](const auto& elem_a, const auto& elem_b) {
                                                return dir_x(elem_a) < dir_x(elem_b);
                                            });
        auto middle2 = std::partition(
            upper_chain_points.begin(), upper_chain_points.end(),
            [&max_point1](const auto& point) { return point.ycoord() < max_point1.ycoord(); });
        auto min_point2 = *std::min_element(lower_chain_points.begin(), lower_chain_points.end(),
                                            [&dir_x](const auto& elem_a, const auto& elem_b) {
                                                return dir_x(elem_a) < dir_x(elem_b);
                                            });
        auto middle3 = std::partition(
            lower_chain_points.begin(), lower_chain_points.end(),
            [&min_point2](const auto& point) { return point.ycoord() > min_point2.ycoord(); });

        std::vector<typename std::iterator_traits<FwIter>::value_type> segment_a_points,
            segment_b_points, segment_c_points, segment_d_points;
        if (vec.x() < 0) {
            segment_a_points.assign(middle3, lower_chain_points.end());
            std::sort(segment_a_points.begin(), segment_a_points.end(),
                      [&dir_x](const auto& a, const auto& b) { return dir_x(a) > dir_x(b); });
            segment_b_points.assign(lower_chain_points.begin(), middle3);
            std::sort(segment_b_points.begin(), segment_b_points.end(),
                      [&dir_y](const auto& a, const auto& b) { return dir_y(a) < dir_y(b); });
            segment_c_points.assign(middle2, upper_chain_points.end());
            std::sort(segment_c_points.begin(), segment_c_points.end(),
                      [&dir_x](const auto& a, const auto& b) { return dir_x(a) < dir_x(b); });
            segment_d_points.assign(upper_chain_points.begin(), middle2);
            std::sort(segment_d_points.begin(), segment_d_points.end(),
                      [&dir_y](const auto& a, const auto& b) { return dir_y(a) > dir_y(b); });
        } else {
            segment_a_points.assign(upper_chain_points.begin(), middle2);
            std::sort(segment_a_points.begin(), segment_a_points.end(),
                      [&dir_x](const auto& a, const auto& b) { return dir_x(a) < dir_x(b); });
            segment_b_points.assign(middle2, upper_chain_points.end());
            std::sort(segment_b_points.begin(), segment_b_points.end(),
                      [&dir_y](const auto& a, const auto& b) { return dir_y(a) < dir_y(b); });
            segment_c_points.assign(lower_chain_points.begin(), middle3);
            std::sort(segment_c_points.begin(), segment_c_points.end(),
                      [&dir_x](const auto& a, const auto& b) { return dir_x(a) > dir_x(b); });
            segment_d_points.assign(middle3, lower_chain_points.end());
            std::sort(segment_d_points.begin(), segment_d_points.end(),
                      [&dir_y](const auto& a, const auto& b) { return dir_y(a) > dir_y(b); });
        }

        std::vector<typename std::iterator_traits<FwIter>::value_type> result;
        result.reserve(segment_a_points.size() + segment_b_points.size() + segment_c_points.size()
                       + segment_d_points.size());
        result.insert(result.end(), segment_a_points.begin(), segment_a_points.end());
        result.insert(result.end(), segment_b_points.begin(), segment_b_points.end());
        result.insert(result.end(), segment_c_points.begin(), segment_c_points.end());
        result.insert(result.end(), segment_d_points.begin(), segment_d_points.end());
        return result;
    }

    // Explicit instantiations for std::vector<Point<int>>::iterator
    template void create_test_rpolygon_old<std::vector<Point<int>>::iterator>(
        std::vector<Point<int>>::iterator&&, std::vector<Point<int>>::iterator&&);

    template auto create_test_rpolygon<std::vector<Point<int>>::iterator>(
        std::vector<Point<int>>::iterator, std::vector<Point<int>>::iterator)
        -> std::vector<Point<int>>;

}
