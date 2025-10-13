#pragma once

#include <algorithm>
#include <cassert>
#include <span>
#include <utility>  // for std::pair
#include <vector>

#include "recti.hpp"

namespace recti {

    /**
     * @brief Polygon
     *
     * `Polygon` is a class template that represents an arbitrary polygon. It
     * stores the origin point and a vector of edges that define the polygon. The
     * template parameter `T` specifies the type of the coordinates of the points.
     *
     * @tparam T
     */
    template <typename T> class Polygon {
      private:
        Point<T> _origin{};
        std::vector<Vector2<T>> _vecs{};

      public:
        /**
         * @brief Default constructor
         */
        constexpr Polygon() = default;

        /**
         * @brief Constructs a new Polygon object from origin and vectors
         *
         * @param[in] origin The origin point of the polygon
         * @param[in] vecs Vector of displacement vectors from origin
         */
        constexpr Polygon(Point<T> origin, std::vector<Vector2<T>> vecs)
            : _origin{std::move(origin)}, _vecs{std::move(vecs)} {}

        /**
         * @brief Constructs a new Polygon object from a set of points.
         *
         * This constructor takes a `std::span` of `Point<T>` objects representing the
         * vertices of the polygon. The first point in the span is used as the origin
         * of the polygon, and the remaining points are used to construct the edges of
         * the polygon as vectors relative to the origin.
         *
         * @param[in] pointset A span of points representing the vertices of the polygon.
         */
        explicit constexpr Polygon(std::span<const Point<T>> pointset) : _origin{pointset.front()} {
            if (pointset.size() <= 1) return;
            _vecs.reserve(pointset.size() - 1);
            for (auto it = pointset.begin() + 1; it != pointset.end(); ++it) {
                _vecs.push_back(*it - _origin);
            }
        }

        /**
         * @brief Equality comparison operator
         */
        constexpr bool operator==(const Polygon& rhs) const {
            return _origin == rhs._origin && _vecs == rhs._vecs;
        }

        /**
         * @brief Inequality comparison operator
         */
        constexpr bool operator!=(const Polygon& rhs) const { return !(*this == rhs); }

        /**
         * @brief Adds a vector to the origin of the polygon, effectively translating the
         * polygon.
         *
         * @param[in] rhs The vector to add to the origin.
         * @return A reference to the modified polygon.
         */
        constexpr auto operator+=(const Vector2<T>& rhs) -> Polygon& {
            this->_origin += rhs;
            return *this;
        }

        /**
         * @brief Subtracts a vector from the origin of the polygon, effectively translating the
         * polygon.
         *
         * @param[in] rhs The vector to subtract from the origin.
         * @return A reference to the modified polygon.
         */
        constexpr auto operator-=(const Vector2<T>& rhs) -> Polygon& {
            this->_origin -= rhs;
            return *this;
        }

        /**
         * @brief Calculates the signed area of the polygon multiplied by 2.
         *
         * This function calculates the signed area of the polygon by summing the
         * cross products of adjacent edges. The result is multiplied by 2 to
         * avoid the need for floating-point arithmetic.
         *
         * @return The signed area of the polygon multiplied by 2.
         */
        constexpr auto signed_area_x2() const -> T {
            if (_vecs.size() < 2) return T{0};

            T res = _vecs[0].x() * _vecs[1].y() - _vecs.back().x() * _vecs[_vecs.size() - 2].y();
            for (size_t i = 1; i < _vecs.size() - 1; ++i) {
                res += _vecs[i].x() * (_vecs[i + 1].y() - _vecs[i - 1].y());
            }
            return res;
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
         * @brief Checks if the polygon is rectilinear.
         *
         * A polygon is rectilinear if all its edges are either horizontal or
         * vertical.
         *
         * @return true if the polygon is rectilinear, false otherwise.
         */
        constexpr auto is_rectilinear() const -> bool {
            // Create a pointset with all vertices relative to origin
            std::vector<Vector2<T>> pointset;
            pointset.reserve(_vecs.size() + 1);
            pointset.emplace_back(0, 0);
            pointset.insert(pointset.end(), _vecs.begin(), _vecs.end());

            // Check all consecutive edges
            for (size_t i = 0; i < pointset.size(); ++i) {
                size_t next = (i + 1) % pointset.size();
                const auto& v1 = pointset[i];
                const auto& v2 = pointset[next];

                if (v1.x() != v2.x() && v1.y() != v2.y()) {
                    return false;
                }
            }

            return true;
        }

        /**
         * @brief Checks if the polygon is convex.
         *
         * A polygon is convex if all its interior angles are less than or equal to 180 degrees.
         *
         * @return true if the polygon is convex, false otherwise.
         */
        constexpr auto is_convex() const -> bool {
            if (_vecs.size() < 2) return false;
            if (_vecs.size() == 2) return true;  // Triangle is convex

            // Create a pointset with all vertices relative to origin
            std::vector<Vector2<T>> pointset;
            pointset.reserve(_vecs.size() + 1);
            pointset.emplace_back(0, 0);
            pointset.insert(pointset.end(), _vecs.begin(), _vecs.end());

            // Determine initial cross product sign
            const auto& pv0 = pointset[pointset.size() - 2];
            const auto& pv2 = pointset[1];
            T cross_product_sign = -pv0.x() * pv2.y() + pv0.y() * pv2.x();

            // Check all consecutive edges
            for (size_t i = 1; i < pointset.size() - 1; ++i) {
                const auto& v0 = pointset[i - 1];
                const auto& v1 = pointset[i];
                const auto& v2 = pointset[i + 1];

                T current_cross_product
                    = (v1.x() - v0.x()) * (v2.y() - v1.y()) - (v1.y() - v0.y()) * (v2.x() - v1.x());

                if ((cross_product_sign > 0) != (current_cross_product > 0)) {
                    return false;
                }
            }

            return true;
        }
    };

    /**
     * @brief Writes the vertices of a polygon to an output stream in a format suitable for
     * rendering.
     *
     * This function writes the vertices of the given polygon to the provided output stream in a
     * format that can be used to render the polygon, such as in a vector graphics format like SVG
     * or LaTeX. Each vertex is written on a new line, prefixed with "\\draw " to indicate that it
     * should be rendered as a drawing command.
     *
     * @tparam Stream The type of the output stream to write to.
     * @tparam T The numeric type used to represent the coordinates of the polygon vertices.
     * @param out The output stream to write the polygon vertices to.
     * @param poly The polygon to write to the output stream.
     * @return The output stream, for method chaining.
     */
    // template <class Stream, typename T> auto operator<<(Stream &out, const Polygon<T> &poly)
    //     -> Stream & {
    //     for (auto &&vtx : poly) {
    //         out << "  \\draw " << vtx << ";\n";
    //     }
    //     return out;
    // }

    /**
     * @brief Create a monotone polygon from a range of points.
     *
     * This function takes a range of points represented by the iterators `first` and `last`, and a
     * comparison function `dir` that defines the order of the points. It then creates a monotone
     * polygon from the points by partitioning them into two halves based on their position relative
     * to the line connecting the minimum and maximum points in the range. The two halves are then
     * sorted in ascending order using the provided comparison function, and the second half is
     * reversed.
     *
     * @tparam FwIter The type of the forward iterator over the points.
     * @tparam Compare The type of the comparison function for the points.
     * @param first The beginning of the range of points.
     * @param last The end of the range of points.
     * @param dir The comparison function for the points.
     */
    template <typename FwIter, typename Compare>
    inline void create_mono_polygon(FwIter&& first, FwIter&& last, Compare&& dir) {
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

    /**
     * @brief Create a xmono Polygon object
     *
     * This function creates a monotone polygon from a range of points represented by the iterators
     * `first` and `last`. It uses a comparison function that compares the points based on their x
     * and y coordinates.
     *
     * @tparam FwIter The type of the forward iterator over the points.
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     */
    template <typename FwIter> inline auto create_xmono_polygon(FwIter&& first, FwIter&& last)
        -> void {
        return create_mono_polygon(first, last, [](const auto& lhs, const auto& rhs) -> bool {
            return std::make_pair(lhs.xcoord(), lhs.ycoord())
                   < std::make_pair(rhs.xcoord(), rhs.ycoord());
        });
    }

    /**
     * @brief Create a ymono Polygon object
     *
     * This function creates a monotone polygon from a range of points represented by the iterators
     * `first` and `last`. It uses a comparison function that compares the points based on their y
     * and x coordinates.
     *
     * @tparam FwIter The type of the forward iterator over the points.
     * @param[in] first The beginning of the range of points.
     * @param[in] last The end of the range of points.
     */
    template <typename FwIter> inline auto create_ymono_polygon(FwIter&& first, FwIter&& last)
        -> void {
        return create_mono_polygon(first, last, [](const auto& lhs, const auto& rhs) -> bool {
            return std::make_pair(lhs.ycoord(), lhs.xcoord())
                   < std::make_pair(rhs.ycoord(), rhs.xcoord());
        });
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
    inline auto polygon_is_monotone(std::span<const Point<T>> pointset, DirFunc&& dir) -> bool {
        if (pointset.size() <= 3) {
            return true;
        }

        const size_t n = pointset.size();

        // Find min and max points according to the direction function using std::minmax_element
        auto [min_it, max_it] = std::minmax_element(
            pointset.begin(), pointset.end(),
            [&dir](const Point<T>& a, const Point<T>& b) { return dir(a) < dir(b); });

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
    template <typename T> inline auto polygon_is_xmonotone(std::span<const Point<T>> pointset)
        -> bool {
        auto x_key
            = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.xcoord(), pt.ycoord()}; };
        return polygon_is_monotone(pointset, x_key);
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
    template <typename T> inline auto polygon_is_ymonotone(std::span<const Point<T>> pointset)
        -> bool {
        auto y_key
            = [](const Point<T>& pt) -> std::pair<T, T> { return {pt.ycoord(), pt.xcoord()}; };
        return polygon_is_monotone(pointset, y_key);
    }

    /**
     * @brief Determine if a point is within a polygon
     *
     * This function determines whether a given point is strictly inside, strictly outside, or on
     * the boundary of a polygon represented by a range of points. It uses the Winding Number
     * algorithm to make this determination.
     *
     * The code below is from Wm. Randolph Franklin <wrf@ecse.rpi.edu>
     * (see URL below) with some minor modifications for integer. It returns
     * true for strictly interior points, false for strictly exterior, and ub
     * for points on the boundary.  The boundary behavior is complex but
     * determined; in particular, for a partition of a region into polygons,
     * each Point is "in" exactly one Polygon.
     * (See p.243 of [O'Rourke (C)] for a discussion of boundary behavior.)
     *
     * See http://www.faqs.org/faqs/graphics/algorithms-faq/ Subject 2.03
     *
     * @tparam T The type of the coordinates of the points in the polygon
     * @param pointset The range of points representing the polygon
     * @param ptq The point to test
     * @return true if the point is strictly inside the polygon, false if the point is strictly
     * outside the polygon
     */
    template <typename T>
    inline auto point_in_polygon(std::span<const Point<T>> pointset, const Point<T>& ptq) -> bool {
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
                } else {  // v1.ycoord() < v0.ycoord()
                    if (det > 0) {
                        res = !res;
                    }
                }
            }
            pt0 = pt1;
        }
        return res;
    }

    /**
     * @brief Determines if a polygon represented by a range of points is oriented anticlockwise.
     *
     * @tparam T The type of the coordinates of the points in the polygon.
     * @param pointset The range of points representing the polygon.
     * @return true if the polygon is oriented anti-clockwise, false otherwise.
     */
    template <typename T> inline auto polygon_is_anticlockwise(std::span<const Point<T>> pointset)
        -> bool {
        const auto it1 = std::min_element(pointset.begin(), pointset.end());
        const auto it0 = it1 != pointset.begin() ? std::prev(it1) : std::prev(pointset.end());
        const auto it2 = std::next(it1) != pointset.end() ? std::next(it1) : pointset.begin();
        return (*it1 - *it0).cross(*it2 - *it1) > 0;
    }
}  // namespace recti
