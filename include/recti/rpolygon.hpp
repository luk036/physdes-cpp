#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <span>
#include <utility>  // for std::pair
#include <utility>
#include <vector>

#include "polygon.hpp"
#include "recti.hpp"

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
        constexpr bool operator==(const RPolygon &rhs) const {
            return _origin == rhs._origin && _vecs == rhs._vecs;
        }

        /**
         * @brief Inequality comparison operator
         */
        constexpr bool operator!=(const RPolygon &rhs) const { return !(*this == rhs); }

        /**
         * @brief Adds a vector to the origin of the rectilinear polygon.
         *
         * This method adds the given vector to the origin point of the rectilinear polygon,
         * effectively translating the entire polygon by the specified vector.
         *
         * @param[in] rhs The vector to add to the origin.
         * @return A reference to the modified RPolygon object.
         */
        constexpr auto operator+=(const Vector2<T> &rhs) -> RPolygon & {
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
        constexpr auto operator-=(const Vector2<T> &rhs) -> RPolygon & {
            this->_origin -= rhs;
            return *this;
        }

        /**
         * @brief Gets the origin point of the polygon
         */
        constexpr const Point<T> &origin() const { return _origin; }

        /**
         * @brief Gets the displacement vectors of the polygon
         */
        constexpr const std::vector<Vector2<T>> &vectors() const { return _vecs; }

        /**
         * @brief Gets all vertices of the polygon as points
         */
        constexpr auto vertices() const -> std::vector<Point<T>> {
            std::vector<Point<T>> result;
            result.reserve(_vecs.size() + 1);
            result.push_back(_origin);
            for (const auto &vec : _vecs) {
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

            for (const auto &next_pt : _vecs) {
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
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     * @param[in] dir The key function that extracts the x and y coordinates of each point.
     * @return `true` if the resulting RPolygon is anti-clockwise, `false` otherwise.
     */
    template <typename FwIter, typename KeyFn>
    inline auto create_mono_rpolygon(FwIter &&first, FwIter &&last, KeyFn &&dir) -> bool {
        assert(first != last);

        // Use x-monotone as model
        auto leftward
            = [&dir](const auto &rhs, const auto &lhs) -> bool { return dir(rhs) < dir(lhs); };
        auto result = std::minmax_element(first, last, leftward);
        const auto leftmost = *result.first;
        const auto rightmost = *result.second;

        const auto is_anticw = dir(rightmost).second <= dir(leftmost).second;
        auto r2l = [&leftmost, &dir](const auto &elem) -> bool {
            return dir(elem).second <= dir(leftmost).second;
        };
        auto l2r = [&leftmost, &dir](const auto &elem) -> bool {
            return dir(elem).second >= dir(leftmost).second;
        };
        const auto middle = is_anticw ? std::partition(first, last, std::move(r2l))
                                      : std::partition(first, last, std::move(l2r));
        std::sort(first, middle, leftward);
        std::sort(middle, last, std::move(leftward));
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
    template <typename FwIter> inline auto create_xmono_rpolygon(FwIter &&first, FwIter &&last)
        -> bool {
        return create_mono_rpolygon(
            first, last, [](const auto &pt) { return std::make_pair(pt.xcoord(), pt.ycoord()); });
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
    template <typename FwIter> inline auto create_ymono_rpolygon(FwIter &&first, FwIter &&last)
        -> bool {
        return create_mono_rpolygon(
            first, last, [](const auto &pt) { return std::make_pair(pt.ycoord(), pt.xcoord()); });
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
    template <typename FwIter> inline void create_test_rpolygon(FwIter &&first, FwIter &&last) {
        assert(first != last);

        auto upwd = [](const auto &rhs, const auto &lhs) -> bool {
            return std::make_pair(rhs.ycoord(), rhs.xcoord())
                   < std::make_pair(lhs.ycoord(), lhs.xcoord());
        };
        auto down = [](const auto &rhs, const auto &lhs) -> bool {
            return std::make_pair(rhs.ycoord(), rhs.xcoord())
                   > std::make_pair(lhs.ycoord(), lhs.xcoord());
        };
        auto left = [](const auto &rhs, const auto &lhs) {
            return std::make_pair(rhs.xcoord(), rhs.ycoord())
                   < std::make_pair(lhs.xcoord(), lhs.ycoord());
        };
        auto right = [](const auto &rhs, const auto &lhs) {
            return std::make_pair(rhs.xcoord(), rhs.ycoord())
                   > std::make_pair(lhs.xcoord(), lhs.ycoord());
        };

        auto result = std::minmax_element(first, last, upwd);
        auto min_pt = *result.first;
        auto max_pt = *result.second;
        auto d_x = max_pt.xcoord() - min_pt.xcoord();
        auto d_y = max_pt.ycoord() - min_pt.ycoord();
        auto middle = std::partition(first, last, [&min_pt, &d_x, &d_y](const auto &elem) -> bool {
            return d_x * (elem.ycoord() - min_pt.ycoord())
                   < (elem.xcoord() - min_pt.xcoord()) * d_y;
        });
        auto max_pt1 = *std::max_element(first, middle, left);
        auto middle2 = std::partition(first, middle, [&max_pt1](const auto &elem) -> bool {
            return elem.ycoord() < max_pt1.ycoord();
        });
        auto min_pt2 = *std::min_element(middle, last, left);
        auto middle3 = std::partition(middle, last, [&min_pt2](const auto &elem) -> bool {
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
    inline auto rpolygon_is_monotone(std::span<const Point<T>> pointset, DirFunc &&dir) -> bool {
        if (pointset.size() <= 3) {
            return true;
        }

        const size_t n = pointset.size();

        // Find min and max points according to the direction function using std::minmax_element
        auto [min_it, max_it] = std::minmax_element(
            pointset.begin(), pointset.end(),
            [&dir](const Point<T> &a, const Point<T> &b) { return dir(a) < dir(b); });

        size_t min_index = static_cast<size_t>(std::distance(pointset.begin(), min_it));
        size_t max_index = static_cast<size_t>(std::distance(pointset.begin(), max_it));

        // Check chain from min to max (should be non-decreasing)
        size_t i = min_index;
        while (i != max_index) {
            size_t next_i = (i + 1) % n;
            auto current_key = dir(pointset[i]);
            auto next_key = dir(pointset[next_i]);

            // Compare the first element of the key tuple (the main direction component)
            if (current_key.first > next_key.first) {
                return false;
            }
            i = next_i;
        }

        // Check chain from max to min (should be non-increasing)
        i = max_index;
        while (i != min_index) {
            size_t next_i = (i + 1) % n;
            auto current_key = dir(pointset[i]);
            auto next_key = dir(pointset[next_i]);

            // Compare the first element of the key tuple (the main direction component)
            if (current_key.first < next_key.first) {
                return false;
            }
            i = next_i;
        }

        return true;
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
            = [](const Point<T> &pt) -> std::pair<T, T> { return {pt.xcoord(), pt.ycoord()}; };
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
            = [](const Point<T> &pt) -> std::pair<T, T> { return {pt.ycoord(), pt.xcoord()}; };
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
    inline auto point_in_rpolygon(std::span<const Point<T>> pointset, const Point<T> &ptq) -> bool {
        auto pt0 = pointset.back();
        const auto &qy = ptq.ycoord();
        const auto &p0y = pt0.ycoord();

        auto res = false;
        for (const auto &pt1 : pointset) {
            const auto &p1y = pt1.ycoord();
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
        auto it1 = std::min_element(pointset.begin(), pointset.end());
        auto it0 = it1 != pointset.begin() ? std::prev(it1) : std::prev(pointset.end());
        return it0->ycoord() > it1->ycoord();
    }
}  // namespace recti
