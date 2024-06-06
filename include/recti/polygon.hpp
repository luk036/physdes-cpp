#pragma once

#include <algorithm>
#include <gsl/span>
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
         * @brief Constructs a new Polygon object from a set of points.
         *
         * This constructor takes a `gsl::span` of `Point<T>` objects representing the
         * vertices of the polygon. The first point in the span is used as the origin
         * of the polygon, and the remaining points are used to construct the edges of
         * the polygon as vectors relative to the origin.
         *
         * @param[in] pointset A span of points representing the vertices of the polygon.
         */
        explicit constexpr Polygon(gsl::span<const Point<T>> pointset) : _origin{pointset.front()} {
            auto itr = pointset.begin();
            for (++itr; itr != pointset.end(); ++itr) {
                this->_vecs.push_back(*itr - this->_origin);
            }
        }

        /**
         * @brief Adds a vector to the origin of the polygon, effectively translating the
         * polygon.
         *
         * @param[in] rhs The vector to add to the origin.
         * @return A reference to the modified polygon.
         */
        constexpr auto operator+=(const Vector2<T> &rhs) -> Polygon & {
            this->_origin += rhs;
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
            auto itr2 = this->_vecs.begin();
            auto itr0 = itr2++;
            auto itr1 = itr2++;
            auto end = this->_vecs.end();
            auto last = std::prev(end);
            auto res = itr0->x() * itr1->y() - last->x() * std::prev(last)->y();
            for (; itr2 != end; ++itr2, ++itr1, ++itr0) {
                res = std::move(res) + itr1->x() * (itr2->y() - itr0->y());
            }
            return res;
        }

        /**
         * @brief
         *
         * @return Point<T>
         */
        auto lb() const -> Point<T>;

        /**
         * @brief
         *
         * @return Point<T>
         */
        auto ub() const -> Point<T>;
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
    template <class Stream, typename T>
    auto operator<<(Stream &out, const Polygon<T> &poly) -> Stream & {
        for (auto &&vtx : poly) {
            out << "  \\draw " << vtx << ";\n";
        }
        return out;
    }

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
    inline void create_mono_polygon(FwIter &&first, FwIter &&last, Compare &&dir) {
        assert(first != last);

        auto result = std::minmax_element(first, last, dir);
        auto min_pt = *result.first;
        auto max_pt = *result.second;
        auto displace = max_pt - min_pt;
        auto middle = std::partition(first, last, [&displace, &min_pt](const auto &elem) -> bool {
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
    template <typename FwIter>
    inline auto create_xmono_polygon(FwIter &&first, FwIter &&last) -> void {
        return create_mono_polygon(first, last, [](const auto &lhs, const auto &rhs) -> bool {
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
    template <typename FwIter>
    inline auto create_ymono_polygon(FwIter &&first, FwIter &&last) -> void {
        return create_mono_polygon(first, last, [](const auto &lhs, const auto &rhs) -> bool {
            return std::make_pair(lhs.ycoord(), lhs.xcoord())
                   < std::make_pair(rhs.ycoord(), rhs.xcoord());
        });
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
    inline auto point_in_polygon(gsl::span<const Point<T>> pointset, const Point<T> &ptq) -> bool {
        auto res = false;
        auto pt0 = pointset.back();
        for (auto &&pt1 : pointset) {
            if ((pt1.ycoord() <= ptq.ycoord() && ptq.ycoord() < pt0.ycoord())
                || (pt0.ycoord() <= ptq.ycoord() && ptq.ycoord() < pt1.ycoord())) {
                auto det = (ptq - pt0).cross(pt1 - pt0);
                if (pt1.ycoord() > pt0.ycoord()) {
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
     * @brief Determines if a polygon represented by a range of points is oriented clockwise.
     *
     * @tparam T The type of the coordinates of the points in the polygon.
     * @param pointset The range of points representing the polygon.
     * @return true if the polygon is oriented clockwise, false otherwise.
     */
    template <typename T>
    inline auto polygon_is_clockwise(gsl::span<const Point<T>> pointset) -> bool {
        auto it1 = std::min_element(pointset.begin(), pointset.end());
        auto it0 = it1 != pointset.begin() ? std::prev(it1) : std::prev(pointset.end());
        auto it2 = std::next(it1) != pointset.end() ? std::next(it1) : pointset.begin();
        return (*it1 - *it0).cross(*it2 - *it1) < 0;
    }
}  // namespace recti
