#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <span>
#include <utility>  // for std::pair
#include <vector>

#include "polygon.hpp"
#include "rdllist.hpp"

namespace recti {

    /**
     * @brief Rectilinear Polygon
     *
     * `RPolygon` is a class template that represents a rectilinear polygon. It
     * stores the origin point and a vector of edges that define the polygon. The
     * template parameter `T` specifies the type of the coordinates of the points.
     *
     * @tparam T
     */
    template <typename T> class RPolygon {
      private:
        Point<T> _origin{};
        std::vector<Vector2<T>> _vecs{};

      public:
        /**
         * @brief Default constructor
         */
        constexpr RPolygon() = default;

        /**
         * @brief Constructs a new RPolygon object from origin and vectors
         *
         * @param[in] origin The origin point of the polygon
         * @param[in] vecs Vector of displacement vectors from origin
         */
        constexpr RPolygon(Point<T> origin, std::vector<Vector2<T>> vecs)
            : _origin{std::move(origin)}, _vecs{std::move(vecs)} {}

        /**
         * @brief Constructs a new RPolygon object from a set of points.
         *
         * This constructor takes a `std::span` of `Point<T>` objects representing the
         * vertices of the rectilinear polygon. The first point in the span is used as
         * the origin of the polygon, and the remaining points are used to construct the
         * vectors that define the edges of the polygon.
         *
         * @param[in] pointset A span of `Point<T>` objects representing the vertices of
         * the rectilinear polygon.
         */
        explicit constexpr RPolygon(std::span<const Point<T>> pointset)
            : _origin{pointset.front()} {
            if (pointset.size() <= 1) return;
            _vecs.reserve(pointset.size() - 1);
            for (auto it = pointset.begin() + 1; it != pointset.end(); ++it) {
                _vecs.emplace_back(*it - _origin);
            }
        }

        /**
         * @brief Equality comparison operator
         */
        constexpr bool operator==(const RPolygon& rhs) const {
            return _origin == rhs._origin && _vecs == rhs._vecs;
        }

        /**
         * @brief Inequality comparison operator
         */
        constexpr bool operator!=(const RPolygon& rhs) const { return !(*this == rhs); }

        /**
         * @brief Adds a vector to the origin of the rectilinear polygon.
         *
         * This method adds the given vector to the origin point of the rectilinear polygon,
         * effectively translating the entire polygon by the specified vector.
         *
         * @param[in] rhs The vector to add to the origin.
         * @return A reference to the modified RPolygon object.
         */
        constexpr auto operator+=(const Vector2<T>& rhs) -> RPolygon& {
            this->_origin += rhs;
            return *this;
        }

        /**
         * @brief Subtracts a vector from the origin of the rectilinear polygon.
         *
         * This method subtracts the given vector from the origin point of the rectilinear polygon,
         * effectively translating the entire polygon by the negative of the specified vector.
         *
         * @param[in] rhs The vector to subtract from the origin.
         * @return A reference to the modified RPolygon object.
         */
        constexpr auto operator-=(const Vector2<T>& rhs) -> RPolygon& {
            this->_origin -= rhs;
            return *this;
        }

        /**
         * @brief Gets the origin point of the polygon
         */
        constexpr const Point<T>& origin() const { return _origin; }

        /**
         * @brief Gets the displacement vectors of the polygon
         */
        constexpr const std::vector<Vector2<T>>& vectors() const { return _vecs; }

        /**
         * @brief Gets all vertices of the polygon as points
         */
        constexpr auto vertices() const -> std::vector<Point<T>> {
            std::vector<Point<T>> result;
            result.reserve(_vecs.size() + 1);
            result.push_back(_origin);
            for (const auto& vec : _vecs) {
                result.push_back(_origin + vec);
            }
            return result;
        }

        /**
         * @brief Calculates the signed area of the rectilinear polygon.
         *
         * This method calculates the signed area of the rectilinear polygon represented by this
         * `RPolygon` object using the shoelace formula.
         *
         * @return The signed area of the rectilinear polygon.
         */
        constexpr auto signed_area() const -> T {
            if (_vecs.empty()) return T{0};

            auto it = _vecs.begin();
            Vector2<T> vec0 = *it++;
            T res = vec0.x() * vec0.y();

            for (; it != _vecs.end(); ++it) {
                Vector2<T> vec1 = *it;
                res += vec1.x() * (vec1.y() - vec0.y());
                vec0 = vec1;
            }
            return res;
        }

        /**
         * @brief Converts the rectilinear polygon to a general polygon
         *
         * This method adds intermediate points to ensure the polygon remains rectilinear
         * when converted to a general polygon representation.
         *
         * @return A Polygon object representing the converted polygon
         */
        constexpr auto to_polygon() const -> Polygon<T> {
            if (_vecs.empty()) {
                return Polygon<T>(_origin, {});
            }

            std::vector<Vector2<T>> new_vecs;
            Vector2<T> current_pt(0, 0);

            for (const auto& next_pt : _vecs) {
                if (current_pt.x() != next_pt.x() && current_pt.y() != next_pt.y()) {
                    // Add intermediate point for non-rectilinear segment
                    new_vecs.emplace_back(next_pt.x(), current_pt.y());
                }
                new_vecs.push_back(next_pt);
                current_pt = next_pt;
            }

            // Handle closing segment
            Vector2<T> first_pt(0, 0);
            if (current_pt.x() != first_pt.x() && current_pt.y() != first_pt.y()) {
                new_vecs.emplace_back(first_pt.x(), current_pt.y());
            }

            return Polygon<T>(_origin, std::move(new_vecs));
        }

        /**
         * @brief Checks if the polygon is rectilinear (all edges are horizontal or vertical)
         */
        constexpr auto is_rectilinear() const -> bool { return true; }
    };

    /**
     * @brief Create a x-monotone rectilinear polygon (RPolygon) object.
     *
     * This function takes a range of points represented by iterators `first` and `last`, and a key
     * function `dir` that extracts the x and y coordinates of each point. It then creates an
     * monotone RPolygon object from the given points.
     *
     * @tparam FwIter The iterator type for the range of points.
     * @tparam KeyFn The type of the key function that extracts the x and y coordinates of each
     * point.
     * @tparam CmpFn The type of the comparison function for the y-coordinates.
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     * @param[in] dir The key function that extracts the x and y coordinates of each point.
     * @param[in] cmp The comparison function for the y-coordinates.
     * @return `true` if the resulting RPolygon is anti-clockwise, `false` otherwise.
     */
    template <typename FwIter, typename KeyFn, typename CmpFn>
    inline auto create_mono_rpolygon(FwIter&& first, FwIter&& last, const KeyFn& dir,
                                     const CmpFn& cmp) -> bool {
        assert(first != last);

        // Use x-monotone as model
        // Lambda for comparing elements based on the provided direction function.
        auto compare_by_direction
            = [&dir](const auto& rhs, const auto& lhs) -> bool { return dir(rhs) < dir(lhs); };
        auto result = std::minmax_element(first, last, compare_by_direction);
        const auto leftmost = *result.first;
        const auto rightmost = *result.second;

        const auto is_anticw = cmp(dir(leftmost).second, dir(rightmost).second);
        // Lambda to check if an element belongs to the right-to-left chain.
        auto is_right_to_left_chain = [&leftmost, &dir](const auto& elem) -> bool {
            return dir(elem).second <= dir(leftmost).second;
        };
        // Lambda to check if an element belongs to the left-to-right chain.
        auto is_left_to_right_chain = [&leftmost, &dir](const auto& elem) -> bool {
            return dir(elem).second >= dir(leftmost).second;
        };
        const auto middle = is_anticw
                                ? std::partition(first, last, std::move(is_right_to_left_chain))
                                : std::partition(first, last, std::move(is_left_to_right_chain));
        std::sort(first, middle, compare_by_direction);
        std::sort(middle, last, std::move(compare_by_direction));
        std::reverse(middle, last);
        return is_anticw;  // is_clockwise if y-monotone
    }

    /**
     * @brief Create a x-monotone rectilinear polygon (RPolygon) object.
     *
     * This function takes a range of points represented by iterators `first` and `last`, and
     * creates an x-monotone RPolygon object from the given points.
     *
     * @tparam FwIter The iterator type for the range of points.
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     * @return `true` if the resulting RPolygon is anti-clockwise, `false` otherwise.
     */
    template <typename FwIter> inline auto create_xmono_rpolygon(FwIter&& first, FwIter&& last)
        -> bool {
        return create_mono_rpolygon(
            first, last, [](const auto& pt) { return std::make_pair(pt.xcoord(), pt.ycoord()); },
            [](const auto& a, const auto& b) -> bool { return a < b; });
    }

    /**
     * @brief Create a y-monotone rectilinear polygon (RPolygon) object.
     *
     * This function takes a range of points represented by iterators `first` and `last`, and
     * creates an y-monotone RPolygon object from the given points.
     *
     * @tparam FwIter The iterator type for the range of points.
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     * @return `true` if the resulting RPolygon is clockwise, `false` otherwise.
     */
    template <typename FwIter> inline auto create_ymono_rpolygon(FwIter&& first, FwIter&& last)
        -> bool {
        return create_mono_rpolygon(
            first, last, [](const auto& pt) { return std::make_pair(pt.ycoord(), pt.xcoord()); },
            [](const auto& a, const auto& b) -> bool { return a > b; });
    }

    /**
     * @brief Create a test rectilinear polygon (RPolygon) object.
     *
     * This function takes a range of points represented by iterators `first` and `last`, and
     * creates a test RPolygon object from the given points. The resulting RPolygon is either
     * clockwise or anti-clockwise depending on the relative positions of the points.
     *
     * @tparam FwIter The iterator type for the range of points.
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     */
    template <typename FwIter> inline void create_test_rpolygon_old(FwIter&& first, FwIter&& last) {
        assert(first != last);

        auto upwd = [](const auto& rhs, const auto& lhs) -> bool {
            return std::make_pair(rhs.ycoord(), rhs.xcoord())
                   < std::make_pair(lhs.ycoord(), lhs.xcoord());
        };
        auto down = [](const auto& rhs, const auto& lhs) -> bool {
            return std::make_pair(rhs.ycoord(), rhs.xcoord())
                   > std::make_pair(lhs.ycoord(), lhs.xcoord());
        };
        auto left = [](const auto& rhs, const auto& lhs) {
            return std::make_pair(rhs.xcoord(), rhs.ycoord())
                   < std::make_pair(lhs.xcoord(), lhs.ycoord());
        };
        auto right = [](const auto& rhs, const auto& lhs) {
            return std::make_pair(rhs.xcoord(), rhs.ycoord())
                   > std::make_pair(lhs.xcoord(), lhs.ycoord());
        };

        auto result = std::minmax_element(first, last, upwd);
        auto min_pt = *result.first;
        auto max_pt = *result.second;
        auto d_x = max_pt.xcoord() - min_pt.xcoord();
        auto d_y = max_pt.ycoord() - min_pt.ycoord();
        auto middle = std::partition(first, last, [&min_pt, &d_x, &d_y](const auto& elem) -> bool {
            return d_x * (elem.ycoord() - min_pt.ycoord())
                   < (elem.xcoord() - min_pt.xcoord()) * d_y;
        });
        auto max_pt1 = *std::max_element(first, middle, left);
        auto middle2 = std::partition(first, middle, [&max_pt1](const auto& elem) -> bool {
            return elem.ycoord() < max_pt1.ycoord();
        });
        auto min_pt2 = *std::min_element(middle, last, left);
        auto middle3 = std::partition(middle, last, [&min_pt2](const auto& elem) -> bool {
            return elem.ycoord() > min_pt2.ycoord();
        });

        if (d_x < 0) {  // clockwise
            std::sort(first, middle2, down);
            std::sort(middle2, middle, left);
            std::sort(middle, middle3, upwd);
            std::sort(middle3, last, right);
        } else {  // anti-clockwise
            std::sort(first, middle2, left);
            std::sort(middle2, middle, upwd);
            std::sort(middle, middle3, right);
            std::sort(middle3, last, down);
        }
    }

    /**
     * @brief Create a test rectilinear polygon (RPolygon) object.
     *
     * This function takes a range of points represented by iterators `first` and `last`, and
     * creates a test RPolygon object from the given points. The resulting RPolygon is either
     * clockwise or anti-clockwise depending on the relative positions of the points.
     *
     * @tparam FwIter The iterator type for the range of points.
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     * @return A vector of points representing the test rectilinear polygon.
     */
    template <typename FwIter> inline auto create_test_rpolygon(FwIter first, FwIter last)
        -> std::vector<typename std::iterator_traits<FwIter>::value_type> {
        using T = typename std::iterator_traits<FwIter>::value_type::value_type;
        assert(first != last);

        auto dir_x = [](const auto& pt) { return std::make_pair(pt.xcoord(), pt.ycoord()); };
        auto dir_y = [](const auto& pt) { return std::make_pair(pt.ycoord(), pt.xcoord()); };

        auto max_pt = *std::max_element(
            first, last, [&dir_y](const auto& a, const auto& b) { return dir_y(a) < dir_y(b); });
        auto min_pt = *std::min_element(
            first, last, [&dir_y](const auto& a, const auto& b) { return dir_y(a) < dir_y(b); });
        Vector2<T> vec = max_pt - min_pt;

        std::vector<typename std::iterator_traits<FwIter>::value_type> upper_chain_points,
            lower_chain_points;
        auto middle = std::partition(
            first, last, [&min_pt, &vec](const auto& pt) { return vec.cross(pt - min_pt) < 0; });
        upper_chain_points.assign(first, middle);
        lower_chain_points.assign(middle, last);

        auto max_pt1 = *std::max_element(
            upper_chain_points.begin(), upper_chain_points.end(),
            [&dir_x](const auto& a, const auto& b) { return dir_x(a) < dir_x(b); });
        auto middle2
            = std::partition(upper_chain_points.begin(), upper_chain_points.end(),
                             [&max_pt1](const auto& pt) { return pt.ycoord() < max_pt1.ycoord(); });
        auto min_pt2 = *std::min_element(
            lower_chain_points.begin(), lower_chain_points.end(),
            [&dir_x](const auto& a, const auto& b) { return dir_x(a) < dir_x(b); });
        auto middle3
            = std::partition(lower_chain_points.begin(), lower_chain_points.end(),
                             [&min_pt2](const auto& pt) { return pt.ycoord() > min_pt2.ycoord(); });

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

        // Find min and max indices
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

        auto violate = [&pointset, &dir](Dllink<size_t>* vi, Dllink<size_t>* v_stop,
                                         std::function<bool(T, T)> cmp) -> bool {
            auto current = vi;
            while (current != v_stop) {
                auto vnext = current->next;
                auto current_key = dir(pointset[current->data]);
                auto next_key = dir(pointset[vnext->data]);
                if (cmp(std::get<0>(current_key), std::get<0>(next_key))) {
                    return true;
                }
                current = vnext;
            }
            return false;
        };

        // Chain from min to max
        if (violate(&v_min, &v_max, [](T a, T b) { return a > b; })) {
            return false;
        }

        // Chain from max to min
        return !violate(&v_max, &v_min, [](T a, T b) { return a < b; });
    }

    /**
     * @brief Check if a polygon is x-monotone
     *
     * A polygon is x-monotone if it can be divided into two chains that are both
     * monotone with respect to the x-axis.
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @return true if the polygon is x-monotone, false otherwise
     */
    template <typename T> inline auto rpolygon_is_xmonotone(std::span<const Point<T>> pointset)
        -> bool {
        auto x_key
            = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.xcoord(), pt.ycoord()}; };
        return rpolygon_is_monotone(pointset, x_key);
    }

    /**
     * @brief Check if a polygon is y-monotone
     *
     * A polygon is y-monotone if it can be divided into two chains that are both
     * monotone with respect to the y-axis.
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @return true if the polygon is y-monotone, false otherwise
     */
    template <typename T> inline auto rpolygon_is_ymonotone(std::span<const Point<T>> pointset)
        -> bool {
        auto y_key
            = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.ycoord(), pt.xcoord()}; };
        return rpolygon_is_monotone(pointset, y_key);
    }

    /**
     * @brief Check if a polygon is convex
     *
     * A rectilinear polygon is convex precisely when it is both x-monotone and y-monotone.
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @return true if the polygon is y-monotone, false otherwise
     */
    template <typename T> inline auto rpolygon_is_convex(std::span<const Point<T>> pointset)
        -> bool {
        return rpolygon_is_xmonotone(pointset) && rpolygon_is_ymonotone(pointset);
    }

    /**
     * @brief Determine if a point is within a rectilinear polygon.
     *
     * This function implements the Wm. Randolph Franklin algorithm to determine if a
     * given point is strictly inside, strictly outside, or on the boundary of a
     * rectilinear polygon defined by the provided set of points.
     *
     * See http://www.faqs.org/faqs/graphics/algorithms-faq/ Subject 2.03
     *
     * @tparam T The numeric type of the point coordinates.
     * @param pointset The set of points defining the rectilinear polygon.
     * @param ptq The point to test for inclusion in the polygon.
     * @return true if the point is strictly inside the polygon, false if the point
     * is strictly outside the polygon, and an unspecified boolean value if the
     * point is on the boundary of the polygon.
     */
    template <typename T>
    inline auto point_in_rpolygon(std::span<const Point<T>> pointset, const Point<T>& ptq) -> bool {
        auto pt0 = pointset.back();
        const auto& qy = ptq.ycoord();
        const auto& p0y = pt0.ycoord();

        auto res = false;
        for (const auto& pt1 : pointset) {
            const auto& p1y = pt1.ycoord();
            if ((p1y <= qy && qy < p0y) || (p0y <= qy && qy < p1y)) {
                if (pt1.xcoord() > ptq.xcoord()) {
                    res = !res;
                }
            }
            pt0 = pt1;
        }
        return res;
    }

    /**
     * @brief Determine if a rectilinear polygon is oriented clockwise.
     *
     * This function takes a span of points defining a rectilinear polygon and
     * determines if the polygon is oriented in a clockwise direction.
     *
     * @tparam T The numeric type of the point coordinates.
     * @param pointset The set of points defining the rectilinear polygon.
     * @return true if the polygon is oriented anti-clockwise, false otherwise.
     */
    template <typename T> inline auto rpolygon_is_anticlockwise(std::span<const Point<T>> pointset)
        -> bool {
        const auto it1 = std::min_element(pointset.begin(), pointset.end());
        const auto it0 = it1 != pointset.begin() ? std::prev(it1) : std::prev(pointset.end());
        return it0->ycoord() > it1->ycoord();
    }
}  // namespace recti
