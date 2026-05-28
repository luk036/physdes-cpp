#pragma once

#include <span>
#include <vector>

#include "point.hpp"

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
                _vecs.emplace_back(*it - _origin);
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
            result.emplace_back(_origin);
            for (const auto& vec : _vecs) {
                result.emplace_back(_origin + vec);
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
            if (_vecs.empty()) return true;

            // Check from origin (0,0) to first vec
            if (_vecs[0].x() != 0 && _vecs[0].y() != 0) return false;

            // Check consecutive vecs directly (no vector allocation)
            for (size_t i = 0; i < _vecs.size() - 1; ++i) {
                if (_vecs[i].x() != _vecs[i + 1].x()
                    && _vecs[i].y() != _vecs[i + 1].y()) {
                    return false;
                }
            }

            // Check closing: last vec back to origin (0,0)
            if (_vecs.back().x() != 0 && _vecs.back().y() != 0) return false;

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
            if (_vecs.size() == 2) return true;

            // pointset[i] maps to: _vecs[i-1] for i>0, or (0,0) for i=0
            // cross_product_sign uses _vecs[N-2] and _vecs[0]
            T cross_product_sign
                = -_vecs[_vecs.size() - 2].x() * _vecs[0].y()
                  + _vecs[_vecs.size() - 2].y() * _vecs[0].x();

            for (size_t i = 0; i < _vecs.size() - 1; ++i) {
                auto v0 = (i == 0) ? Vector2<T>(0, 0) : _vecs[i - 1];
                const auto& v1 = _vecs[i];
                const auto& v2 = _vecs[i + 1];

                T current_cross = (v1.x() - v0.x()) * (v2.y() - v1.y())
                                - (v1.y() - v0.y()) * (v2.x() - v1.x());

                if ((cross_product_sign > 0) != (current_cross > 0)) {
                    return false;
                }
            }

            return true;
        }
    };

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
    template <typename FwIter> auto create_xmono_polygon(FwIter first, FwIter last) -> void;

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
    template <typename FwIter> auto create_ymono_polygon(FwIter first, FwIter last) -> void;

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
    auto polygon_is_monotone(std::span<const Point<T>> pointset, const DirFunc& dir) -> bool;

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
    template <typename T> auto polygon_is_xmonotone(std::span<const Point<T>> pointset) -> bool;

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
    template <typename T> auto polygon_is_ymonotone(std::span<const Point<T>> pointset) -> bool;

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
    auto point_in_polygon(std::span<const Point<T>> pointset, const Point<T>& ptq) -> bool;

    /**
     * @brief Determines if a polygon represented by a range of points is oriented anticlockwise.
     *
     * @tparam T The type of the coordinates of the points in the polygon.
     * @param pointset The range of points representing the polygon.
     * @return true if the polygon is oriented anti-clockwise, false otherwise.
     */
    template <typename T> auto polygon_is_anticlockwise(std::span<const Point<T>> pointset) -> bool;
}  // namespace recti
