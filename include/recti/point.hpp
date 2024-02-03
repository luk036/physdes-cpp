#pragma once

#include <tuple>    // import std::tie()
#include <utility>  // import std::move

#include "interval.hpp"
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
     * @tparam T2 int or Interval
     */
    template <typename T1 = int, typename T2 = T1> class Point {
        using Self = Point<T1, T2>;

        /**
         * @brief
         *
         * @tparam U1
         * @tparam U2
         */
        template <typename U1, typename U2> friend class Point;

        T1 _xcoord;  //!< xcoord coordinate
        T2 _ycoord;  //!< ycoord coordinate

      public:
        /**
         * @brief Construct a new Point object
         *
         * This is a constructor for the `Point` class. It takes two parameters,
         * `xcoord` and `ycoord`, and constructs a new `Point` object with those
         * values. The parameters are passed as rvalue references (`T1&&` and `T2&&`)
         * to allow for efficient move semantics. The constructor is marked as
         * `constexpr` to indicate that it can be evaluated at compile-time if the
         * arguments are compile-time constants. The `noexcept` specifier indicates
         * that the constructor does not throw any exceptions.
         *
         * @param[in] xcoord
         * @param[in] ycoord
         */
        constexpr Point(T1 &&xcoord, T2 &&ycoord) noexcept : _xcoord{xcoord}, _ycoord{ycoord} {}

        /**
         * @brief Construct a new Point object
         *
         * @param[in] xcoord
         * @param[in] ycoord
         */
        constexpr Point(const T1 &xcoord, const T2 &ycoord) : _xcoord{xcoord}, _ycoord{ycoord} {}

        /**
         * @brief
         *
         * @return const T1&
         */
        constexpr auto xcoord() const -> const T1 & { return this->_xcoord; }

        /**
         * @brief
         *
         * @return const T2&
         */
        constexpr auto ycoord() const -> const T2 & { return this->_ycoord; }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * @brief Equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        constexpr auto operator==(const Point<U1, U2> &rhs) const -> bool {
            return std::tie(this->xcoord(), this->ycoord()) == std::tie(rhs.xcoord(), rhs.ycoord());
        }

        /**
         * @brief Less than
         *
         * @tparam U1
         * @tparam U2
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto operator<(const Point<U1, U2> &rhs) const -> bool {
            return std::tie(this->xcoord(), this->ycoord()) < std::tie(rhs.xcoord(), rhs.ycoord());
        }

        /**
         * @brief Not equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        constexpr auto operator!=(const Point<U1, U2> &rhs) const -> bool {
            return !(*this == rhs);
        }

        /**
         * @brief Greater than
         *
         * @tparam U1
         * @tparam U2
         * @param[in] xcoord
         * @param[in] ycoord
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        friend constexpr auto operator>(const Self &xcoord, const Point<U1, U2> &ycoord) -> bool {
            return ycoord < xcoord;
        }

        /**
         * @brief Less than or equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] xcoord
         * @param[in] ycoord
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        friend constexpr auto operator<=(const Self &xcoord, const Point<U1, U2> &ycoord) -> bool {
            return !(ycoord < xcoord);
        }

        /**
         * @brief Greater than or equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] xcoord
         * @param[in] ycoord
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        friend constexpr auto operator>=(const Self &xcoord, const Point<U1, U2> &ycoord) -> bool {
            return !(xcoord < ycoord);
        }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

        /**
         * @brief Add a vector (translation)
         *
         * @tparam U1
         * @tparam U2
         * @param[in] rhs
         * @return Self&
         */
        template <typename U1, typename U2> CONSTEXPR14 auto operator+=(const Vector2<U1, U2> &rhs)
            -> Self & {
            this->_xcoord += rhs.x();
            this->_ycoord += rhs.y();
            return *this;
        }

        /**
         * @brief Substract a vector (translation)
         *
         * @tparam U1
         * @tparam U2
         * @param[in] rhs
         * @return Self&
         */
        template <typename U1, typename U2> CONSTEXPR14 auto operator-=(const Vector2<U1, U2> &rhs)
            -> Self & {
            this->_xcoord -= rhs.x();
            this->_ycoord -= rhs.y();
            return *this;
        }

        /**
         * @brief Add
         *
         * @tparam U1
         * @tparam U2
         * @param[in] p
         * @param[in] v
         * @return Vector2<T>
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator+(Point lhs, const Vector2<U1, U2> &vec2) {
            auto xcoord = lhs.xcoord() + vec2.x();
            auto ycoord = lhs.ycoord() + vec2.y();
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * @brief Substract
         *
         * @tparam U1
         * @tparam U2
         * @param[in] xcoord
         * @param[in] ycoord
         * @return Vector2<T>
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator-(Point lhs, const Vector2<U1, U2> &vec2) -> Point {
            return lhs -= vec2;
        }

        /**
         * @brief Add a vector (translation)
         *
         * @param[in] alpha
         * @return Self&
         */
        CONSTEXPR14 auto operator+=(const T2 &alpha) -> Self & {
            this->_xcoord += alpha;
            this->_ycoord += alpha;
            return *this;
        }

        /**
         * @brief Substract
         *
         * @param[in] alpha
         * @return Self&
         */
        CONSTEXPR14 auto operator-=(const T1 &alpha) -> Self & {
            this->_xcoord -= alpha;
            this->_ycoord -= alpha;
            return *this;
        }

        /**
         * @brief Add
         *
         * @param[in] xcoord
         * @param[in] alpha
         * @return Point
         */
        friend constexpr auto operator+(Point lhs, const T1 &alpha) -> Point {
            return lhs += alpha;
        }

        /**
         * @brief Substract
         *
         * @param[in] xcoord
         * @param[in] alpha
         * @return Point
         */
        friend constexpr auto operator-(Point lhs, const T1 &alpha) -> Point {
            return lhs -= alpha;
        }

        /**
         * @brief Different
         *
         * @param[in] rhs
         * @return constexpr auto
         */
        constexpr auto operator-(const Self &other) const {
            auto xcoord = this->xcoord() - other.xcoord();
            auto ycoord = this->ycoord() - other.ycoord();
            return Vector2<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                               std::move(ycoord)};
        }

        /**
         * @brief flip_xcoordy according to xcoord-ycoord diagonal line
         *
         * @return Point<T2, T1>
         */
        constexpr auto flip_xy() const -> Point<T2, T1> { return {this->ycoord(), this->xcoord()}; }

        /**
         * @brief flip according to ycoord-axis
         *
         * @return Point<T2, T1>
         */
        constexpr auto flip_y() const -> Point<T1, T2> { return {-this->xcoord(), this->ycoord()}; }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto overlaps(const Point<U1, U2> &other) const -> bool {
            return overlap(this->xcoord(), other.xcoord())
                   && overlap(this->ycoord(), other.ycoord());
        }

        /**
         * @brief intersection
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto intersection_with(const Point<U1, U2> &other) const {
            auto xcoord = intersection(this->xcoord(), other.xcoord());
            auto ycoord = intersection(this->ycoord(), other.ycoord());
            return Point<decltype(xcoord), decltype(ycoord)>{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * @brief
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto contains(const Point<U1, U2> &other) const -> bool {
            return contain(this->xcoord(), other.xcoord())
                   && contain(this->ycoord(), other.ycoord());
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto min_dist_with(const Point<U1, U2> &other) const {
            return min_dist(this->xcoord(), other.xcoord())
                   + min_dist(this->ycoord(), other.ycoord());
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto min_dist_change_with(Point<U1, U2> &other) {
            return min_dist_change(this->_xcoord, other._xcoord)
                   + min_dist_change(this->_ycoord, other._ycoord);
        }

        template <typename R>  //
        friend constexpr auto enlarge(const Point &lhs, const R &alpha) {
            auto xcoord = enlarge(lhs.xcoord(), alpha);
            auto ycoord = enlarge(lhs.ycoord(), alpha);
            return Point{std::move(xcoord), std::move(ycoord)};
        }

        /**
         * @brief
         *
         * @tparam T1
         * @tparam T2
         * @tparam Stream
         * @param[out] out
         * @param[in] p
         * @return Stream&
         */
        template <class Stream> friend auto operator<<(Stream &out, const Point &obj) -> Stream & {
            out << "(" << obj.xcoord() << ", " << obj.ycoord() << ")";
            return out;
        }

      protected:
        /**
         * @brief
         *
         * @return const T1&
         */
        CONSTEXPR14 auto get_xcoord() -> T1 & { return this->_xcoord; }

        /**
         * @brief
         *
         * @return const T2&
         */
        CONSTEXPR14 auto get_ycoord() -> T2 & { return this->_ycoord; }
    };
#pragma pack(pop)

#pragma pack(push, 1)
    /**
     * @brief 2D Point
     *
     * @tparam T1
     * @tparam T2
     */
    template <typename T1, typename T2 = T1> class dualpoint : public Point<T1, T2> {
      public:
        /**
         * @brief
         *
         * @return const T1&
         */
        constexpr auto ycoord() const -> const T1 &  // override intentionally
        {
            return this->_xcoord;
        }

        /**
         * @brief
         *
         * @return const T2&
         */
        constexpr auto xcoord() const -> const T2 &  // override intentionally
        {
            return this->_ycoord;
        }
    };
#pragma pack(pop)

    /**
     * @brief adapter for containers of Point (deprecated)
     *
     * @tparam iter
     */
    template <typename iterator> class dual_iterator : public iterator {
        using value_type = typename iterator::value_type;
        using T1 = decltype(std::declval(iterator::value_type).xcoord());
        using T2 = decltype(std::declval(iterator::value_type).ycoord());

        constexpr explicit dual_iterator(iterator &&itr) : iterator{std::forward<iterator>(itr)} {}

        CONSTEXPR14 auto operator*() const noexcept -> const dualpoint<T2, T1> & {
            return dualpoint<T2, T1>{};
            // return std::reinterpret_cast<const dualpoint<T2,
            // T1>&>(*iterator::operator*());
        }

        CONSTEXPR14 auto operator*() noexcept -> dualpoint<T2, T1> & {
            return dualpoint<T2, T1>{};
            // return std::reinterpret_cast<dualpoint<T2,
            // T1>&>(*iterator::operator*());
        }
    };
}  // namespace recti
