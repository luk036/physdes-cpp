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
     * @code{.txt}
     * +-----------------------------+
     * |        RPolygon<T>          |
     * +-----------------------------+
     * | _origin: Point<T>           |
     * | _vecs: vector<Vector2<T>>   |
     * |                             |
     * |        +----+               |
     * |        |    |               |  A rectilinear polygon class
     * |    +---+    |               |  with origin point and
     * |    |   |    |               |  vectors defining the polygon
     * |    |   +----+               |  edges. Supports various
     * |    |                        |  geometric operations
     * |    +------------------------+
     * |            vertices()       |
     * |            signed_area()    |
     * |            to_polygon()     |
     * +-----------------------------+
     * @endcode
     *
     * @tparam T
     */
    template <typename T> class RPolygon {
      private:
        Point<T> _origin{};               ///< Origin point of the polygon
        std::vector<Vector2<T>> _vecs{};  ///< Vectors defining the polygon edges

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
            : _origin{origin}, _vecs{std::move(vecs)} {}

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
            for (auto iterator = pointset.begin() + 1; iterator != pointset.end(); ++iterator) {
                _vecs.emplace_back(*iterator - _origin);
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
         * @param[in] vector_offset The vector to add to the origin.
         * @return A reference to the modified RPolygon object.
         */
        constexpr auto operator+=(const Vector2<T>& vector_offset) -> RPolygon& {
            this->_origin += vector_offset;
            return *this;
        }

        /**
         * @brief Subtracts a vector from the origin of the rectilinear polygon.
         *
         * This method subtracts the given vector from the origin point of the rectilinear polygon,
         * effectively translating the entire polygon by the negative of the specified vector.
         *
         * @param[in] vector_offset The vector to subtract from the origin.
         * @return A reference to the modified RPolygon object.
         */
        constexpr auto operator-=(const Vector2<T>& vector_offset) -> RPolygon& {
            this->_origin -= vector_offset;
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

            auto iterator = _vecs.begin();
            Vector2<T> first_vec = *iterator++;
            T result = first_vec.x() * first_vec.y();

            for (; iterator != _vecs.end(); ++iterator) {
                Vector2<T> second_vec = *iterator;
                result += second_vec.x() * (second_vec.y() - first_vec.y());
                first_vec = second_vec;
            }
            return result;
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
            Vector2<T> current_point(0, 0);

            for (const auto& next_point : _vecs) {
                if (current_point.x() != next_point.x() && current_point.y() != next_point.y()) {
                    // Add intermediate point for non-rectilinear segment
                    new_vecs.emplace_back(next_point.x(), current_point.y());
                }
                new_vecs.push_back(next_point);
                current_point = next_point;
            }

            // Handle closing segment
            Vector2<T> first_point(0, 0);
            if (current_point.x() != first_point.x() && current_point.y() != first_point.y()) {
                new_vecs.emplace_back(first_point.x(), current_point.y());
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
    auto create_mono_rpolygon(FwIter&& first, FwIter&& last, const KeyFn& dir, const CmpFn& cmp)
        -> bool;

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
    template <typename FwIter>
    auto create_xmono_rpolygon(FwIter&& first, FwIter&& last) -> bool;

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
    template <typename FwIter>
    auto create_ymono_rpolygon(FwIter&& first, FwIter&& last) -> bool;

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
    template <typename FwIter>
    void create_test_rpolygon_old(FwIter&& first, FwIter&& last);

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
    template <typename FwIter>
    auto create_test_rpolygon(FwIter first, FwIter last)
        -> std::vector<typename std::iterator_traits<FwIter>::value_type>;

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
    template <typename T>
    auto rpolygon_is_xmonotone(std::span<const Point<T>> pointset) -> bool;

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
    template <typename T>
    auto rpolygon_is_ymonotone(std::span<const Point<T>> pointset) -> bool;

    /**
     * @brief Check if a polygon is convex
     *
     * A rectilinear polygon is convex precisely when it is both x-monotone and y-monotone.
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @return true if the polygon is y-monotone, false otherwise
     */
    template <typename T>
    auto rpolygon_is_convex(std::span<const Point<T>> pointset) -> bool;

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
    auto point_in_rpolygon(std::span<const Point<T>> pointset, const Point<T>& query_point) -> bool;

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
    template <typename T>
    auto rpolygon_is_anticlockwise(std::span<const Point<T>> pointset) -> bool;
}  // namespace recti
