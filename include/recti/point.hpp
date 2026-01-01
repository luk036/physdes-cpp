#pragma once

#include <tuple>    // for std::tie()
#include <utility>  // for std::move

#include "generic.hpp"
#include "vector2.hpp"

namespace recti {

    template <typename T1, typename T2> class Point;

#pragma pack(push, 1)
    /**
     * @brief Point
     *
     * The `Point` class is a template class that represents a point in a 2D
     * coordinate system. It has two template parameters, `T1` and `T2`, which can
     * be either `int`, `Interval`, or another `Point`. The `Point` class provides
     * various operations and functionalities for working with points, such as
     * comparison operators, arithmetic operators, flipping, overlap checking,
     * distance calculation, and more.
     *
     * @tparam T1 int, Interval, or Point
     * @tparam T2 int, Interval, or Point
     */
    template <typename T1 = int, typename T2 = T1> class Point {
        using Self = Point<T1, T2>;

        /**
         * @brief Allow all other specializations to access private members
         */
        /// \cond INTERNAL
        template <typename, typename> friend class Point;
        /// \endcond

        T1 _xcoord;  //!< x coordinate
        T2 _ycoord;  //!< y coordinate

      public:
        using value_type = T1;

        /**
         * @brief Default constructor
         *
         */
        constexpr Point() : _xcoord{}, _ycoord{} {}

        /**
         * @brief Construct a new Point object
         *
         * This is a constructor for the `Point` class. It takes two parameters,
         * `xcoord` and `ycoord`, and constructs a new `Point` object with those
         * values. The parameters are passed as rvalue references (`T1&&` and `T2&&`)
         * to allow for efficient move semantics. The constructor is marked as
         * `constexpr` to indicate that it can be evaluated at compile-time if the
         * arguments are compile-time constants.
         *
         * @param[in] xcoord - x coordinate.
         * @param[in] ycoord - y coordinate.
         */
        constexpr Point(T1 xcoord, T2 ycoord) noexcept
            : _xcoord{std::move(xcoord)}, _ycoord{std::move(ycoord)} {}



        /**
         * Gets the x coordinate of this Point.
         *
         * @return Const reference to the x coordinate.
         */
        constexpr auto xcoord() const -> const T1& { return this->_xcoord; }

        /**
         * Gets the y coordinate of this Point.
         *
         * @return Const reference to the y coordinate.
         */
        constexpr auto ycoord() const -> const T2& { return this->_ycoord; }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * Compares this Point object with another Point object for equality.
         *
         * This operator checks if the x and y coordinates of this Point are equal
         * to the x and y coordinates of the given Point object. It allows Points
         * to be compared for equality in a generic way.
         *
         * @tparam U1 - The type of x coordinate for this Point.
         * @tparam U2 - The type of y coordinate for this Point.
         * @param[in] rhs - The other Point object to compare against.
         * @return True if the two Point objects have equal x and y coordinates, false otherwise.
         */
        template <typename U1, typename U2>
        constexpr auto operator==(const Point<U1, U2>& rhs) const -> bool {
            return std::tie(this->xcoord(), this->ycoord()) == std::tie(rhs.xcoord(), rhs.ycoord());
        }

        /**
         * Compares this Point object with another Point object to check if it is less than.
         *
         * This operator checks if this Point is less than the given Point by comparing
         * their x and y coordinates. It allows Points to be compared in a generic way.
         *
         * @tparam U1 - The type of x coordinate for this Point.
         * @tparam U2 - The type of y coordinate for this Point.
         * @param[in] rhs - The other Point to compare against.
         * @return True if this Point is less than rhs, false otherwise.
         */
        template <typename U1, typename U2>  //
        constexpr auto operator<(const Point<U1, U2>& rhs) const -> bool {
            return std::tie(this->xcoord(), this->ycoord()) < std::tie(rhs.xcoord(), rhs.ycoord());
        }

        /**
         * Compares this Point object with another Point object for equality.
         *
         * This operator checks if the x and y coordinates of this Point are not equal
         * to the x and y coordinates of the given Point object. It allows Points
         * to be compared for equality in a generic way.
         *
         * @tparam U1 - The type of x coordinate for this Point.
         * @tparam U2 - The type of y coordinate for this Point.
         * @param[in] rhs - The other Point object to compare against.
         * @return false if the two Point objects have equal x and y coordinates, true otherwise.
         */
        template <typename U1, typename U2>
        constexpr auto operator!=(const Point<U1, U2>& rhs) const -> bool {
            return !(*this == rhs);
        }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

        /**
         * Adds a vector (translation) to this Point.
         *
         * Translates this Point by the given vector's x and y components.
         *
         * @tparam U1 - The x coordinate type of this Point.
         * @tparam U2 - The y coordinate type of this Point.
         * @param[in] right_point - The vector to translate this Point by.
         * @return Reference to this Point after translation.
         */
        template <typename U1, typename U2> CONSTEXPR14 auto operator+=(const Vector2<U1, U2>& right_point)
            -> Self& {
            this->_xcoord += right_point.x();
            this->_ycoord += right_point.y();
            return *this;
        }

        /**
         * Adds a vector (translation) to this Point.
         *
         * Translates this Point by the given vector's x and y components.
         *
         * @tparam U1 - The x coordinate type of this Point.
         * @tparam U2 - The y coordinate type of this Point.
         * @param[in] vector - The vector to translate this Point by.
         * @return A new Point after translation.
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator+(Point lhs, const Vector2<U1, U2>& vector) {
            auto xcoord = lhs.xcoord() + vector.x();
            auto ycoord = lhs.ycoord() + vector.y();
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * Subtracts a vector (translation) from this Point.
         *
         * Translates this Point by the given vector's x and y components.
         *
         * @tparam U1 - The x coordinate type of this Point.
         * @tparam U2 - The y coordinate type of this Point.
         * @param[in] right_point - The vector to translate this Point by.
         * @return Reference to this Point after translation.
         */
        template <typename U1, typename U2> CONSTEXPR14 auto operator-=(const Vector2<U1, U2>& right_point)
            -> Self& {
            this->_xcoord -= right_point.x();
            this->_ycoord -= right_point.y();
            return *this;
        }

        /**
         * Subtracts a vector (translation) from this Point.
         *
         * Translates this Point by the given vector's x and y components.
         *
         * @tparam U1 - The x coordinate type of this Point.
         * @tparam U2 - The y coordinate type of this Point.
         * @param[in] vector - The vector to translate this Point by.
         * @return A new Point after translation.
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator-(Point lhs, const Vector2<U1, U2>& vector) {
            auto xcoord = lhs.xcoord() - vector.x();
            auto ycoord = lhs.ycoord() - vector.y();
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * @brief Calculates the displacement vector between this point and another point
         *
         * @param[in] other_point The other point to calculate the displacement from
         * @return The displacement vector from other_point to this point
         */
        constexpr auto operator-(const Self& other_point) const {
            auto xcoord = this->xcoord() - other_point.xcoord();
            auto ycoord = this->ycoord() - other_point.ycoord();
            return Vector2<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                               std::move(ycoord)};
        }

        /**
         * @brief measure (area, volume etc.)
         *
         * The function returns the measure (area, volume etc.).
         */
        constexpr auto measure() const {
            return measure_of(this->xcoord()) * measure_of(this->ycoord());
        }

        /**
         * @brief flip_xy according to xcoord-ycoord diagonal line
         *
         * @return Point<T2, T1>
         */
        constexpr auto flip_xy() const -> Point<T2, T1> { return {this->ycoord(), this->xcoord()}; }

        /**
         * @brief flip according to ycoord-axis
         *
         * @return Point<T1, T2>
         */
        constexpr auto flip_y() const -> Point<T1, T2> { return {-this->xcoord(), this->ycoord()}; }

        /**
         * Checks if this point overlaps with another point.
         *
         * @tparam U1 - The type of the x-coordinate for this point.
         * @tparam U2 - The type of the y-coordinate for this point.
         * @param other_point - The other point to check for overlap.
         * @return true if the x and y coordinates overlap, false otherwise.
         */
        template <typename U1, typename U2>  //
        constexpr auto overlaps(const Point<U1, U2>& other_point) const -> bool {
            return overlap(this->xcoord(), other_point.xcoord())
                   && overlap(this->ycoord(), other_point.ycoord());
        }

        /**
         * Checks if this point intersects with another point.
         *
         * @tparam U1 - The x-coordinate type of the other point.
         * @tparam U2 - The y-coordinate type of the other point.
         * @param other_point - The other point to check for intersection.
         * @return The intersection point if the points intersect.
         */
        template <typename U1, typename U2>  //
        constexpr auto intersect_with(const Point<U1, U2>& other_point) const {
            auto xcoord = intersection(this->xcoord(), other_point.xcoord());
            auto ycoord = intersection(this->ycoord(), other_point.ycoord());
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * Return the hull (bounding box) of this point and another point.
         *
         * @tparam U1 - The x-coordinate type of the other point.
         * @tparam U2 - The y-coordinate type of the other point.
         * @param other_point - The other point to check for hull.
         * @return The hull "point".
         */
        template <typename U1, typename U2>  //
        constexpr auto hull_with(const Point<U1, U2>& other_point) const {
            auto xcoord = hull(this->xcoord(), other_point.xcoord());
            auto ycoord = hull(this->ycoord(), other_point.ycoord());
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * Checks if this point contains another point.
         *
         * @tparam U1 - The type of the x-coordinate for this point.
         * @tparam U2 - The type of the y-coordinate for this point.
         * @param other_point - The other point to check for containment.
         * @return true if the x and y coordinates contain, false otherwise.
         */
        template <typename U1, typename U2>  //
        constexpr auto contains(const Point<U1, U2>& other_point) const -> bool {
            return contain(this->xcoord(), other_point.xcoord())
                   && contain(this->ycoord(), other_point.ycoord());
        }

        /**
         * Checks if this point blocks another point.
         *
         * @tparam U1 - The type of the x-coordinate for this point.
         * @tparam U2 - The type of the y-coordinate for this point.
         * @param other_point - The other point to check for containment.
         * @return true if the x and y coordinates contain, false otherwise.
         */
        template <typename U1, typename U2>  //
        constexpr auto blocks(const Point<U1, U2>& other_point) const -> bool {
            return (contain(this->xcoord(), other_point.xcoord())
                    && contain(other_point.ycoord(), this->ycoord()))
                   || (contain(this->ycoord(), other_point.ycoord())
                       && contain(other_point.xcoord(), this->xcoord()));
        }

        /**
         * @brief minimum distance between this point and another point
         *
         * @tparam U1 - The x-coordinate type of the other point.
         * @tparam U2 - The y-coordinate type of the other point.
         * @param other_point - The other point to calculate the minimum distance from.
         * @return The minimum distance between this point and the other point.
         */
        template <typename U1, typename U2>  //
        constexpr auto min_dist_with(const Point<U1, U2>& other_point) const {
            return min_dist(this->xcoord(), other_point.xcoord())
                   + min_dist(this->ycoord(), other_point.ycoord());
        }

        /**
         * @brief minimum distance between this point and another point
         *
         * @tparam U1 - The x-coordinate type of the other point.
         * @tparam U2 - The y-coordinate type of the other point.
         * @param other_point - The other point to calculate the minimum distance from.
         * @return The minimum distance between this point and the other point.
         */
        template <typename U1, typename U2>  //
        constexpr auto min_dist_change_with(Point<U1, U2>& other_point) {
            return min_dist_change(this->_xcoord, other_point._xcoord)
                   + min_dist_change(this->_ycoord, other_point._ycoord);
        }

        /**
         * @brief Enlarge the point by `value`
         *
         * @param[in] value The value to enlarge
         * @return A reference to the modified `Interval` object.
         */
        template <typename T> constexpr auto enlarge_with(const T& value) const {
            auto xb = enlarge(this->xcoord(), value);
            auto yb = enlarge(this->ycoord(), value);
            return Point<decltype(xb), decltype(yb)>{std::move(xb), std::move(yb)};
        }

        /**
         * Return the point in this object that is nearest to the given point.
         *
         * @param other_point - The other point.
         * @return The nearest "point".
         */
        template <typename U1, typename U2>  //
        constexpr auto nearest_to(const Point<U1, U2>& other_point) const {
            auto xcoord = nearest(this->xcoord(), other_point.xcoord());
            auto ycoord = nearest(this->ycoord(), other_point.ycoord());
            return Point<U1, U2>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * @brief Rotate the point by 45 degrees (used for ManhattanArc).
         *
         * @return A new rotated Point.
         */
        constexpr auto rotates() const -> Point<T1, T2> {
            if constexpr (std::is_same_v<T1, Point>) {
                auto pt = _xcoord.rotates();
                auto pt2 = Point(pt.ycoord(), _ycoord).rotates();
                auto pt3 = Point(pt.xcoord(), pt2.ycoord()).rotates();
                return Point(Point(pt3.xcoord(), pt2.xcoord()), pt3.ycoord());
            } else {
                auto xcoord = _xcoord - _ycoord;
                auto ycoord = _xcoord + _ycoord;
                return Point<T1, T2>(xcoord, ycoord);
            }
        }

        /**
         * @brief Inverse rotate the point (used for ManhattanArc).
         *
         * @return A new inversely rotated Point.
         */
        constexpr auto inv_rotates() const -> Point<T1, T2> {
            if constexpr (std::is_same_v<T1, Point>) {
                auto pt = Point(_xcoord.xcoord(), _ycoord).inv_rotates();
                auto pt2 = Point(_xcoord.ycoord(), pt.ycoord()).inv_rotates();
                auto pt3 = Point(pt.xcoord(), pt2.xcoord()).inv_rotates();
                return Point(Point(pt3.xcoord(), pt3.ycoord()), pt2.ycoord());
            } else {
                auto xcoord = (_xcoord + _ycoord) / 2;
                auto ycoord = (-_xcoord + _ycoord) / 2;
                return Point<T1, T2>(xcoord, ycoord);
            }
        }

        /**
         * @brief Calculate the center of the point.
         *
         * @return The center of the point.
         */
        constexpr auto get_center() const {
            auto xcoord = center(_xcoord);
            auto ycoord = center(_ycoord);
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * @brief Calculate the lower corner of the point.
         *
         * @return The lower corner of the point.
         */
        constexpr auto lower_corner() const {
            auto xcoord = lower(_xcoord);
            auto ycoord = lower(_ycoord);
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * @brief Calculate the upper corner of the point.
         *
         * @return The upper corner of the point.
         */
        constexpr auto upper_corner() const {
            auto xcoord = upper(_xcoord);
            auto ycoord = upper(_ycoord);
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * Outputs the point object to the given output stream.
         *
         * @param[out] output_stream The output stream to write the point to.
         * @param[in] obj The point object to output.
         * @return The output stream after writing the point.
         */
        template <class Stream> friend auto operator<<(Stream& output_stream, const Point& obj) -> Stream& {
            output_stream << "(" << obj.xcoord() << ", " << obj.ycoord() << ")";
            return output_stream;
        }

      protected:
        /**
         * @brief Returns a reference to the x-coordinate of the point.
         *
         * @return const T1& A reference to the x-coordinate of the point.
         */
        CONSTEXPR14 auto get_xcoord() -> T1& { return this->_xcoord; }

        /**
         * @brief Returns a reference to the x-coordinate of the point.
         *
         * @return const T1& A reference to the x-coordinate of the point.
         */
        constexpr auto get_xcoord() const -> const T1& { return this->_xcoord; }

        /**
         * @brief Returns a reference to the y-coordinate of the point.
         *
         * @return const T2& A reference to the y-coordinate of the point.
         */
        CONSTEXPR14 auto get_ycoord() -> T2& { return this->_ycoord; }

        /**
         * @brief Returns a reference to the y-coordinate of the point.
         *
         * @return const T2& A reference to the y-coordinate of the point.
         */
        constexpr auto get_ycoord() const -> const T2& { return this->_ycoord; }
    };
#pragma pack(pop)

}  // namespace recti
