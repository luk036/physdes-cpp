#pragma once

#include <algorithm>
#include <span>
#include <utility>  // for std::pair
#include <vector>

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
        std::vector<Vector2<T>> _vecs{};  // @todo: add custom allocator support

      public:
        /**
         * @brief Constructs a new `RPolygon` object from a set of points.
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
            _vecs.reserve(pointset.size() - 1);  // Pre-allocate memory
            for (auto itr = std::next(pointset.begin()); itr != pointset.end(); ++itr) {
                _vecs.emplace_back(*itr - _origin);  // Use emplace_back for in-place construction
            }
        }

        /**
         * @brief Adds a vector to the origin of the rectilinear polygon.
         *
         * This method adds the given vector to the origin point of the rectilinear polygon,
         * effectively translating the entire polygon by the specified vector.
         *
         * @param[in] vector The vector to add to the origin.
         * @return A reference to the modified RPolygon object.
         */
        constexpr auto operator+=(const Vector2<T> &vector) -> RPolygon & {
            this->_origin += vector;
            return *this;
        }

        /**
         * @brief Calculates the signed area of the rectilinear polygon.
         *
         * This method calculates the signed area of the rectilinear polygon represented by this
         * `RPolygon` object.
         *
         * @return The signed area of the rectilinear polygon.
         */
        constexpr auto signed_area() const -> T {
            assert(_vecs.size() >= 1);
            T res = _vecs[0].x() * _vecs[0].y();
            if (_vecs.size() == 1) {
                return res;
            }
            auto prev_y = _vecs[0].y();
            for (auto it = _vecs.begin() + 1; it != _vecs.end(); ++it) {
                res += it->x() * (it->y() - prev_y);
                prev_y = it->y();
            }
            return res;
        }

        /**
         * @brief Checks if the given point is contained within the rectilinear polygon.
         *
         * This method checks if the provided point is contained within the rectilinear polygon
         * represented by this `RPolygon` object.
         *
         * @tparam U The type of the point coordinates.
         * @param[in] rhs The point to check for containment.
         * @return `true` if the point is contained within the rectilinear polygon, `false`
         * otherwise.
         */
        template <typename U> auto contains(const Point<U> &rhs) const -> bool;

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
     * @return true if the polygon is oriented clockwise, false otherwise.
     */
    template <typename T> inline auto rpolygon_is_clockwise(std::span<const Point<T>> pointset)
        -> bool {
        auto it1 = std::min_element(pointset.begin(), pointset.end());
        auto it0 = it1 != pointset.begin() ? std::prev(it1) : pointset.end() - 1;
        if (it1->ycoord() < it0->ycoord()) {
            return false;
        }
        if (it1->ycoord() > it0->ycoord()) {
            return true;
        }
        // it1->ycoord() == it0->ycoord()
        auto it2 = std::next(it1) != pointset.end() ? std::next(it1) : pointset.begin();
        return it2->ycoord() > it1->ycoord();
    }
}  // namespace recti
