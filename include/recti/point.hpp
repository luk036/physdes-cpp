#pragma once

#include <tuple>        // import std::tie()
#include <type_traits>  // import std::is_scalar_v
#include <utility>      // import std::move

#include "interval.hpp"
#include "vector2.hpp"

namespace recti {

    /**
     * @brief Forward declaration
     *
     * @tparam U1
     * @tparam U2
     */
    template <typename U1, typename U2> class point;

#pragma pack(push, 1)
    /**
     * @brief 2D point
     *
     * @tparam T1
     * @tparam T2
     */
    template <typename T1, typename T2 = T1> class point {
        using Self = point<T1, T2>;

        /**
         * @brief
         *
         * @tparam U1
         * @tparam U2
         */
        template <typename U1, typename U2> friend class point;

      protected:
        T1 _x;  //!< x coordinate
        T2 _y;  //!< y coordinate

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

        /**
         * @brief tie
         *
         * @return auto
         */
        constexpr auto _tie() const { return std::tie(this->_x, this->_y); }

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
        template <typename U1, typename U2>  //
        constexpr auto operator<(const point<U1, U2>& rhs) const -> bool {
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
         * @tparam U1
         * @tparam U2
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
         * @tparam U1
         * @tparam U2
         * @param[in] rhs
         * @return Self&
         */
        template <typename U1, typename U2> constexpr auto operator+=(const vector2<U1, U2>& rhs)
            -> Self& {
            this->_x += rhs.x();
            this->_y += rhs.y();
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
        template <typename U1, typename U2> constexpr auto operator-=(const vector2<U1, U2>& rhs)
            -> Self& {
            this->_x -= rhs.x();
            this->_y -= rhs.y();
            return *this;
        }

        /**
         * @brief Add
         *
         * @tparam U1
         * @tparam U2
         * @param[in] p
         * @param[in] v
         * @return vector2<T>
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator+(point p, const vector2<U1, U2>& v) {
            auto x = p.x() + v.x();
            auto y = p.y() + v.y();
            return point<decltype(x), decltype(y)>{std::move(x), std::move(y)};
        }

        /**
         * @brief Substract
         *
         * @tparam U1
         * @tparam U2
         * @param[in] x
         * @param[in] y
         * @return vector2<T>
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator-(point x, const vector2<U1, U2>& y) -> point {
            return x -= y;
        }

        /**
         * @brief Add a vector (translation)
         *
         * @param[in] alpha
         * @return Self&
         */
        constexpr auto operator+=(const T1& alpha) -> Self& {
            this->_x += alpha;
            this->_y += alpha;
            return *this;
        }

        /**
         * @brief Substract
         *
         * @param[in] alpha
         * @return Self&
         */
        constexpr auto operator-=(const T1& alpha) -> Self& {
            this->_x -= alpha;
            this->_y -= alpha;
            return *this;
        }

        /**
         * @brief Add
         *
         * @param[in] x
         * @param[in] alpha
         * @return point
         */
        friend constexpr auto operator+(point x, const T1& alpha) -> point { return x += alpha; }

        /**
         * @brief Substract
         *
         * @param[in] x
         * @param[in] alpha
         * @return point
         */
        friend constexpr auto operator-(point x, const T1& alpha) -> point { return x -= alpha; }

        /**
         * @brief Different
         *
         * @param[in] rhs
         * @return constexpr auto
         */
        constexpr auto operator-(const Self& rhs) const {
            auto x = this->x() - rhs.x();
            auto y = this->y() - rhs.y();
            return vector2<decltype(x), decltype(y)>{std::move(x), std::move(y)};
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

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto overlaps(const point<U1, U2>& other) const -> bool {
            return overlap(this->x(), other.x()) && overlap(this->y(), other.y());
        }

        /**
         * @brief intersection
         *
         * @tparam U1
         * @tparam U2
         * @param other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto intersection_with(const point<U1, U2>& other) const {
            auto x = intersection(this->x(), other.x());
            auto y = intersection(this->y(), other.y());
            return point<decltype(x), decltype(y)>{std::move(x), std::move(y)};
        }

        /**
         * @brief
         *
         * @tparam U1
         * @tparam U2
         * @param other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto contains(const point<U1, U2>& other) const -> bool {
            return contain(this->x(), other.x()) && contain(this->y(), other.y());
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto min_dist_with(const point<U1, U2>& other) const {
            return min_dist(this->x(), other.x()) + min_dist(this->y(), other.y());
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto min_dist_change_with(point<U1, U2>& other) {
            return min_dist_change(this->_x, other._x) + min_dist_change(this->_y, other._y);
        }

        template <typename R>  //
        friend constexpr auto enlarge(const point& lhs, const R& alpha) {
            auto x = enlarge(lhs.x(), alpha);
            auto y = enlarge(lhs.y(), alpha);
            return point<decltype(x), decltype(y)>{std::move(x), std::move(y)};
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
        template <class Stream> friend auto operator<<(Stream& out, const point& p) -> Stream& {
            out << "(" << p.x() << ", " << p.y() << ")";
            return out;
        }
    };
#pragma pack(pop)

#pragma pack(push, 1)
    /**
     * @brief 2D point
     *
     * @tparam T1
     * @tparam T2
     */
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
}  // namespace recti
