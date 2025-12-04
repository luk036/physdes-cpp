#pragma once

#include "interval.hpp"  // for Interval
#include "point.hpp"     // for Point

namespace recti {

    /**
     * @brief Rectangle (Rectilinear)
     *
     * The `struct Rectangle` is inheriting from the `Point<Interval<T>>` class.
     * This means that `Rectangle` will have all the member variables and member
     * functions of `Point<Interval<T>>`. It is using the `Interval<T>` template
     * parameter to define the type of the x and y coordinates of the rectangle.
     *
     * @code{.txt}
     * +-----------------------+
     * |        Recti          |
     * |      Rectangle        |
     * +-----------------------+
     * |                       |
     * |  +----------+         |
     * |  |          |         |  A Rectangle template class
     * |  |   Rect   |         |  inheriting from Point<Interval<T>>
     * |  |          |         |  with functions to get 
     * |  +----------+         |  lower-left (ll) and upper-right (ur)
     * |    ll()----->o--------+  corners, and calculate area
     * |         ur()----->o--------+
     * |                       |    |
     * +-----------------------+    |
     *                             |
     * +-----------------------+    |
     * |    ll() function      |    |
     * | Returns Point<T>      |    |
     * | at lower-left corner  |    |
     * +-----------------------+    |
     *                             |
     * +-----------------------+    |
     * |    ur() function      |    |
     * | Returns Point<T>      |    |
     * | at upper-right corner |    |
     * +-----------------------+    |
     *                             |
     * +-----------------------+    |
     * |     area() function   |    |
     * | Calculates rectangle  |    |
     * | area using length()   |    |
     * +-----------------------+
     * @endcode
     *
     * @tparam T
     */
    template <typename T> struct Rectangle : Point<Interval<T>> {
        /**
         * @brief Construct a new Rectangle object
         *
         * @param[in] xcoord
         * @param[in] ycoord
         */
        constexpr Rectangle(Interval<T> xcoord, Interval<T> ycoord) noexcept
            : Point<Interval<T>>{std::move(xcoord), std::move(ycoord)} {}

        /**
         * @brief Construct a new Rectangle object from the base object (implicitly)
         *
         * Note: intentionally allow implicit conversion
         *
         * @param[in] base
         */
        constexpr Rectangle(
            Point<Interval<T>> base) noexcept  // Note: intentionally allow implicit conversion
            : Point<Interval<T>>{std::move(base)} {}

        /**
         * @brief lower left corner
         *
         * The `ll()` function is a member function of the `Rectangle` struct. It
         * returns a `Point<T>` object representing the lower left corner of the
         * rectangle.
         *
         * @return Point<T>
         */
        constexpr auto ll() const -> Point<T> { return {this->xcoord().lb(), this->ycoord().lb()}; }

        /**
         * @brief upper right corner
         *
         * The `ur()` function is a member function of the `Rectangle` struct. It
         * returns a `Point<T>` object representing the upper right corner of the
         * rectangle. It does this by accessing the `xcoord()` and `ycoord()` member
         * functions of the `Rectangle` object and calling their `ub()` member
         * functions to get the upper bound values. These values are then used to
         * construct a new `Point<T>` object representing the upper right corner.
         *
         * @return Point<T>
         */
        constexpr auto ur() const -> Point<T> { return {this->xcoord().ub(), this->ycoord().ub()}; }

        /**
         * @brief area
         *
         * The `area()` function is a member function of the `Rectangle` struct. It
         * calculates and returns the area of the rectangle.
         *
         * @return constexpr T
         */
        constexpr auto area() const -> T {
            return this->xcoord().length() * this->ycoord().length();
        }
    };

    /**
     * @brief Horizontal Line Segment
     *
     * The `struct HSegment` is defining a horizontal line segment in a rectilinear
     * coordinate system. It is inheriting from the `Point<Interval<T>, T>` class,
     * which means that it will have all the member variables and member functions
     * of `Point<Interval<T>, T>`. The `Interval<T>` template parameter is used to
     * define the type of the x-coordinate of the line segment, and the `T` template
     * parameter is used to define the type of the y-coordinate of the line segment.
     *
     * @tparam T
     */
    template <typename T> struct HSegment : Point<Interval<T>, T> {
        /**
         * @brief Construct a new HSegment object
         *
         * @param[in] xcoord
         * @param[in] ycoord
         */
        constexpr HSegment(Interval<T> xcoord, T ycoord) noexcept
            : Point<Interval<T>, T>{std::move(xcoord), std::move(ycoord)} {}

        /**
         * @brief Construct a new HSegment object from the.
         *
         * @param[in] base
         */
        constexpr HSegment(
            Point<Interval<T>, T> base) noexcept  // Note: intentionally allow implicit conversion
            : Point<Interval<T>, T>{std::move(base)} {}
    };

    /**
     * @brief Vertical Line Segment
     *
     * The `struct VSegment` is defining a vertical line segment in a rectilinear
     * coordinate system. It is inheriting from the `Point<T, Interval<T>>` class,
     * which means that it will have all the member variables and member functions
     * of `Point<T, Interval<T>>`. The `Interval<T>` template parameter is used to
     * define the type of the y-coordinate of the line segment, and the `T` template
     * parameter is used to define the type of the x-coordinate of the line segment.
     *
     * @tparam T
     */
    template <typename T> struct VSegment : Point<T, Interval<T>> {
        /**
         * @brief Construct a new VSegment object
         *
         * @param[in] xcoord
         * @param[in] ycoord
         */
        constexpr VSegment(T xcoord, Interval<T> ycoord) noexcept
            : Point<T, Interval<T>>{std::move(xcoord), std::move(ycoord)} {}

        /**
         * @brief Construct a new VSegment object from the base object (implicitly)
         *
         * @param[in] base
         */
        constexpr VSegment(
            Point<T, Interval<T>> base) noexcept  // Note: intentionally allow implicit conversion
            : Point<T, Interval<T>>{std::move(base)} {}
    };

}  // namespace recti
