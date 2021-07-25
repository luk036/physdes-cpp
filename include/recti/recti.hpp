#pragma once

#include <boost/operators.hpp>
#include <cassert>
// #include <compare>  // for <=> operators
#include <tuple>    // import std::tie()
#include <utility>  // import std::move

#include "vector2.hpp"

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief 2D point
     *
     * @tparam T1
     * @tparam T2
     */
    template <typename T1, typename T2 = T1> class point
        : boost::additive2<point<T1, T2>, vector2<T1>> {
        using Self = point<T1, T2>;

      protected:
        T1 _x;  //!< x coordinate
        T2 _y;  //!< y coordinate

      private:
        auto _tie() const { return std::tie(_x, _y); }

      public:
        /**
         * @brief Construct a new point object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr point(T1&& x, T2&& y) noexcept : _x{std::move(x)}, _y{std::move(y)} {}

        /**
         * @brief Construct a new point object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr point(const T1& x, const T2& y) : _x{x}, _y{y} {}

        /**
         * @brief
         *
         * @return const T1&
         */
        [[nodiscard]] constexpr auto x() const noexcept -> const T1& { return this->_x; }

        /**
         * @brief
         *
         * @return const T2&
         */
        [[nodiscard]] constexpr auto y() const noexcept -> const T2& { return this->_y; }

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
        constexpr auto operator==(const point<U1, U2>& rhs) const -> bool {
            return this->_tie() == rhs._tie();
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
        template <typename U1, typename U2> constexpr auto operator<(const point<U1, U2>& rhs) const
            -> bool {
            return this->_tie() < rhs._tie();
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
        constexpr auto operator!=(const point<U1, U2>& rhs) const -> bool {
            return !(*this == rhs);
        }

        /**
         * @brief Greater than
         *
         * @tparam U1
         * @tparam U2
         * @param[in] x
         * @param[in] y
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        friend constexpr auto operator>(const Self& x, const point<U1, U2>& y) -> bool {
            return y < x;
        }

        /**
         * @brief Less than or equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] x
         * @param[in] y
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        friend constexpr auto operator<=(const Self& x, const point<U1, U2>& y) -> bool {
            return !(y < x);
        }

        /**
         * @brief Greater than or equal to
         *
         * @param[in] x
         * @param[in] y
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        friend constexpr auto operator>=(const Self& x, const point<U1, U2>& y) -> bool {
            return !(x < y);
        }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

        /**
         * @brief Add a vector (translation)
         *
         * @tparam U
         * @param rhs
         * @return Self&
         */
        template <typename U> constexpr auto operator+=(const vector2<U>& rhs) -> Self& {
            this->_x += rhs.x();
            this->_y += rhs.y();
            return *this;
        }

        /**
         * @brief Substract a vector (translation)
         *
         * @tparam U
         * @param rhs
         * @return Self&
         */
        template <typename U> constexpr auto operator-=(const vector2<U>& rhs) -> Self& {
            this->_x -= rhs.x();
            this->_y -= rhs.y();
            return *this;
        }

        /**
         * @brief Different
         *
         * @param rhs
         * @return vector2
         */
        constexpr auto operator-(const Self& rhs) const -> vector2<T1> {
            return {this->x() - rhs.x(), this->y() - rhs.y()};
        }

        /**
         * @brief flip_xy according to x-y diagonal line
         *
         * @return point<T2, T1>
         */
        [[nodiscard]] constexpr auto flip_xy() const -> point<T2, T1> {
            return {this->y(), this->x()};
        }

        /**
         * @brief flip according to y-axis
         *
         * @return point<T2, T1>
         */
        [[nodiscard]] constexpr auto flip_y() const -> point<T1, T2> {
            return {-this->x(), this->y()};
        }
    };
#pragma pack(pop)

    /**
     * @brief
     *
     * @tparam Stream
     * @tparam T1
     * @tparam T2
     * @param[in] out
     * @param[in] p
     * @return Stream&
     */
    template <typename T1, typename T2, class Stream>
    auto operator<<(Stream& out, const point<T1, T2>& p) -> Stream&;

/**
 * @brief 2D point
 *
 * @tparam T1
 * @tparam T2
 */
#pragma pack(push, 1)
    template <typename T1, typename T2 = T1> class dualpoint : public point<T1, T2> {
      public:
        /**
         * @brief
         *
         * @return const T1&
         */
        constexpr auto y() const -> const T1&  // override intentionally
        {
            return this->_x;
        }

        /**
         * @brief
         *
         * @return const T2&
         */
        constexpr auto x() const -> const T2&  // override intentionally
        {
            return this->_y;
        }
    };
#pragma pack(pop)

    /**
     * @brief adapter for containers of point
     *
     * @tparam iter
     */
    template <typename iterator> class dual_iterator : public iterator {
        using value_type = typename iterator::value_type;
        using T1 = decltype(std::declval(iterator::value_type).x());
        using T2 = decltype(std::declval(iterator::value_type).y());

        constexpr dual_iterator(iterator&& a) : iterator{std::forward<iterator>(a)} {}

        constexpr auto operator*() const noexcept -> const dualpoint<T2, T1>& {
            return dualpoint<T2, T1>{};
            // return std::reinterpret_cast<const dualpoint<T2,
            // T1>&>(*iterator::operator*());
        }

        constexpr auto operator*() noexcept -> dualpoint<T2, T1>& {
            return dualpoint<T2, T1>{};
            // return std::reinterpret_cast<dualpoint<T2,
            // T1>&>(*iterator::operator*());
        }
    };

/**
 * @brief Interval
 *
 * @tparam T
 */
#pragma pack(push, 1)
    template <typename T = int> class interval : boost::totally_ordered<interval<T>> {
      private:
        T _lower;  //> lower bound
        T _upper;  //> upper bound

        auto _tie() const { return std::tie(_lower, _upper); }

      public:
        /**
         * @brief Construct a new interval object
         *
         * @param[in] lower
         * @param[in] upper
         */
        constexpr interval(T&& lower, T&& upper) noexcept
            : _lower{std::move(lower)}, _upper{std::move(upper)} {
            assert(!(_upper < _lower));
        }

        /**
         * @brief Construct a new interval object
         *
         * @param[in] lower
         * @param[in] upper
         */
        constexpr interval(const T& lower, const T& upper) : _lower{lower}, _upper{upper} {
            assert(!(_upper < _lower));
        }

        /**
         * @brief
         *
         * @return const T&
         */
        [[nodiscard]] constexpr auto lower() const -> const T& { return this->_lower; }

        /**
         * @brief
         *
         * @return const T&
         */
        [[nodiscard]] constexpr auto upper() const -> const T& { return this->_upper; }

        /**
         * @brief
         *
         * @return constexpr T
         */
        [[nodiscard]] constexpr auto len() const -> T { return this->upper() - this->lower(); }

        /**
         * @brief
         *
         * @param[in] rhs
         * @return true
         * @return false
         */
        constexpr auto operator==(const interval& rhs) const -> bool {
            return this->_tie() == rhs._tie();
        }

        /**
         * @brief
         *
         * @param[in] rhs
         * @return true
         * @return false
         */
        constexpr auto operator<(const interval& rhs) const -> bool {
            return this->upper() < rhs.lower();
        }

        /**
         * @brief
         *
         * @tparam U
         * @param[in] a
         * @return true
         * @return false
         */
        template <typename U> //
        [[nodiscard]] constexpr auto contains(const interval<U>& a) const -> bool {
            return !(a.lower() < this->lower() || this->upper() < a.upper());
        }

        /**
         * @brief
         *
         * @param[in] x
         * @return true
         * @return false
         */
        [[nodiscard]] constexpr auto contains(const T& a) const -> bool {
            return !(a < this->lower() || this->upper() < a);
        }
    };
#pragma pack(pop)

    /**
     * @brief
     *
     * @tparam Stream
     * @tparam T
     * @param[in] out
     * @param[in] p
     * @return Stream&
     */
    template <typename T, class Stream> auto operator<<(Stream& out, const interval<T>& p)
        -> Stream&;

/**
 * @brief Rectangle (Rectilinear)
 *
 * @tparam T
 * @todo use "__attribute__((aligned(0)))" to align struct 'rectangle<int>' to 0
 * bytes
 */
#pragma pack(push, 1)
    template <typename T> struct rectangle : point<interval<T>> {
        /**
         * @brief Construct a new rectangle object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr rectangle(interval<T>&& x, interval<T>&& y) noexcept
            : point<interval<T>>{std::move(x), std::move(y)} {}

        /**
         * @brief Construct a new rectangle object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr rectangle(const interval<T>& x, const interval<T>& y)
            : point<interval<T>>{x, y} {}

        /**
         * @brief
         *
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        [[nodiscard]] constexpr auto contains(const point<U1, U2>& rhs) const -> bool {
            return this->x().contains(rhs.x()) && this->y().contains(rhs.y());
        }

        /**
         * @brief
         *
         * @return point<T>
         */
        [[nodiscard]] constexpr auto lower() const -> point<T> {
            return {this->x().lower(), this->y().lower()};
        }

        /**
         * @brief
         *
         * @return point<T>
         */
        [[nodiscard]] constexpr auto upper() const -> point<T> {
            return {this->x().upper(), this->y().upper()};
        }

        /**
         * @brief
         *
         * @return constexpr T
         */
        [[nodiscard]] constexpr auto area() const -> T { return this->x().len() * this->y().len(); }

        // /**
        //  * @brief
        //  *
        //  * @tparam Stream
        //  * @tparam T
        //  * @param[in] out
        //  * @param[in] r
        //  * @return Stream&
        //  */
        // template <class Stream> friend auto operator<<(Stream& out, const rectangle& r) ->
        // Stream& {
        //     out << r.lower() << " rectangle " << r.upper();
        //     return out;
        // }
    };
#pragma pack(pop)

/**
 * @brief Horizontal Line Segment
 *
 * @tparam T
 * @todo pack
 */
#pragma pack(push, 1)
    template <typename T> struct hsegment : point<interval<T>, T> {
        /**
         * @brief Construct a new hsegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr hsegment(interval<T>&& x, T&& y) noexcept
            : point<interval<T>, T>{std::move(x), std::move(y)} {}

        /**
         * @brief Construct a new hsegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr hsegment(const interval<T>& x, const T& y) : point<interval<T>, T>{x, y} {}

        /**
         * @brief
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U> constexpr auto contains(const point<U>& rhs) const -> bool {
            return this->y() == rhs.y() && this->x().contains(rhs.x());
        }
    };
#pragma pack(pop)

/**
 * @brief vsegment Line Segment
 *
 * @tparam T
 * @todo pack
 */
#pragma pack(push, 1)
    template <typename T> struct vsegment : point<T, interval<T>> {
        /**
         * @brief Construct a new vsegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr vsegment(T&& x, interval<T>&& y) noexcept
            : point<T, interval<T>>{std::move(x), std::move(y)} {}

        /**
         * @brief Construct a new vsegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr vsegment(const T& x, const interval<T>& y) : point<T, interval<T>>{x, y} {}

        /**
         * @brief
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U> constexpr auto contains(const point<U>& rhs) const -> bool {
            return this->x() == rhs.x() && this->y().contains(rhs.y());
        }
    };
#pragma pack(pop)

}  // namespace recti
