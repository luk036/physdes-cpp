#pragma once

#include <algorithm>   // for std::sort
#include <optional>    // for std::optional, std::nullopt
#include <vector>      // for std::vector

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

    /**
     * @brief Detect if any pair of rectangles overlap using the line sweep algorithm.
     *
     * The algorithm uses a sweep line approach:
     * 1. Create events for left and right edges of each rectangle
     * 2. Sort events by x-coordinate
     * 3. Sweep from left to right, maintaining active rectangles
     * 4. Check y-overlap when a new rectangle becomes active
     *
     * @tparam Container A container of Rectangle objects (e.g., std::vector<Rectangle<int>>)
     * @param rectangles The list of rectangles to check for overlaps
     * @return A pair of overlapping rectangles if found, otherwise std::nullopt
     */
    template <typename Container>
    constexpr auto detect_overlap(const Container& rectangles)
        -> std::optional<std::pair<typename Container::value_type, typename Container::value_type>> {
        using RectT = typename Container::value_type;
        using T = typename RectT::value_type;  // Extract the underlying type (int)

        if (rectangles.size() < 2) {
            return std::nullopt;
        }

        using Event = std::tuple<T, int, size_t>;
        std::vector<Event> events;

        size_t idx = 0;
        for (const auto& rect : rectangles) {
            if (rect.xcoord().is_invalid() || rect.ycoord().is_invalid()) {
                ++idx;
                continue;
            }
            events.emplace_back(rect.xcoord().lb(), 1, idx);
            events.emplace_back(rect.xcoord().ub(), -1, idx);
            ++idx;
        }

        std::sort(events.begin(), events.end(),
                  [](const Event& a, const Event& b) { return std::get<0>(a) < std::get<0>(b); });

        std::vector<std::pair<size_t, Interval<T>>> active;

        for (const auto& [x, event_type, rect_idx] : events) {
            const auto& rect = rectangles[rect_idx];

            if (event_type == 1) {
                for (const auto& [other_idx, other_y] : active) {
                    if (rect.ycoord().overlaps(other_y)) {
                        return std::make_optional(
                            std::make_pair(rect, rectangles[other_idx]));
                    }
                }
                active.emplace_back(rect_idx, rect.ycoord());
            } else {
                for (size_t i = 0; i < active.size(); ++i) {
                    if (active[i].first == rect_idx) {
                        active.erase(active.begin() + i);
                        break;
                    }
                }
            }
        }

        return std::nullopt;
    }

}  // namespace recti
