#pragma once

#include <tuple>        // import std::tie()
#include <type_traits>  // import std::is_scalar_v
#include <utility>      // import std::move

#include "vector2.hpp"

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief 2D point
     *
     * @tparam T1
     * @tparam T2
     */
    template <typename T1, typename T2 = T1> class point {
        using Self = point<T1, T2>;

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
         * @brief tie
         * 
         * @return auto 
         */
        constexpr auto _tie() const { return std::tie(_x, _y); }

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
        template <typename U1, typename U2> //
        constexpr auto operator<=>(const point<U1, U2>& rhs) const {
            return this->_tie() <=> rhs._tie();
        }

        // /**
        //  * @brief Not equal to
        //  *
        //  * @tparam U1
        //  * @tparam U2
        //  * @param[in] rhs
        //  * @return true
        //  * @return false
        //  */
        // template <typename U1, typename U2>
        // constexpr auto operator!=(const point<U1, U2>& rhs) const -> bool {
        //     return !(*this == rhs);
        // }

        // /**
        //  * @brief Greater than
        //  *
        //  * @tparam U1
        //  * @tparam U2
        //  * @param[in] x
        //  * @param[in] y
        //  * @return true
        //  * @return false
        //  */
        // template <typename U1, typename U2>
        // friend constexpr auto operator>(const Self& x, const point<U1, U2>& y) -> bool {
        //     return y < x;
        // }

        // /**
        //  * @brief Less than or equal to
        //  *
        //  * @tparam U1
        //  * @tparam U2
        //  * @param[in] x
        //  * @param[in] y
        //  * @return true
        //  * @return false
        //  */
        // template <typename U1, typename U2>
        // friend constexpr auto operator<=(const Self& x, const point<U1, U2>& y) -> bool {
        //     return !(y < x);
        // }

        // /**
        //  * @brief Greater than or equal to
        //  *
        //  * @param[in] x
        //  * @param[in] y
        //  * @return true
        //  * @return false
        //  */
        // template <typename U1, typename U2>
        // friend constexpr auto operator>=(const Self& x, const point<U1, U2>& y) -> bool {
        //     return !(x < y);
        // }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

        /**
         * @brief Add a vector (translation)
         *
         * @tparam U
         * @param[in] rhs
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
         * @param[in] rhs
         * @return Self&
         */
        template <typename U> constexpr auto operator-=(const vector2<U>& rhs) -> Self& {
            this->_x -= rhs.x();
            this->_y -= rhs.y();
            return *this;
        }

        /**
         * @brief Add
         *
         * @tparam U
         * @param[in] x
         * @param[in] y
         * @return vector2<T>
         */
        template <typename U>  //
        friend constexpr auto operator+(point x, const vector2<U>& y) -> point {
            return x += y;
        }

        /**
         * @brief Substract
         *
         * @tparam U
         * @param[in] x
         * @param[in] y
         * @return vector2<T>
         */
        template <typename U>  //
        friend constexpr auto operator-(point x, const vector2<U>& y) -> point {
            return x -= y;
        }

        /**
         * @brief Different
         *
         * @param[in] rhs
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

        /**
         * @brief
         *
         * @tparam U
         * @param[in] a
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto overlaps(const point<U1, U2>& a) const -> bool {
            if constexpr (std::is_scalar_v<T1>) {
                if constexpr (std::is_scalar_v<U1>) {
                    if (this->x() != a.x()) return false;
                } else {
                    if (!a.x().overlaps(this->x())) return false;
                }
            } else {
                if (!this->x().overlaps(a.x())) return false;
            }

            if constexpr (std::is_scalar_v<T2>) {
                if constexpr (std::is_scalar_v<U2>) {
                    if (this->y() != a.y()) return false;
                } else {
                    if (!a.y().overlaps(this->y())) return false;
                }
            } else {
                if (!this->y().overlaps(a.y())) return false;
            }

            return true;
        }

        /**
         * @brief
         *
         * @tparam U
         * @param[in] a
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto contains(const point<U1, U2>& a) const -> bool {
            if constexpr (std::is_scalar_v<T1>) {
                if constexpr (std::is_scalar_v<U1>) {
                    if constexpr (std::is_scalar_v<T2>) {
                        if constexpr (std::is_scalar_v<U2>) {
                            if (this->x() != a.x()) return false;
                            if (this->y() != a.y()) return false;
                            return true;
                        } else {
                            return false;
                        }
                    } else {
                        if (this->x() != a.x()) return false;
                        if (!this->y().contains(a.y())) return false;
                        return true;
                    }
                } else {
                    return false;
                }
            } else {
                if constexpr (std::is_scalar_v<U1>) {
                    if constexpr (std::is_scalar_v<T2>) {
                        if constexpr (std::is_scalar_v<U2>) {
                            if (!this->x().contains(a.x())) return false;
                            if (this->y() != a.y()) return false;
                            return true;
                        } else {
                            return false;
                        }
                    } else {
                        if (!this->x().contains(a.x())) return false;
                        if (!this->y().contains(a.y())) return false;
                        return true;
                    }
                } else {
                    if constexpr (std::is_scalar_v<T2>) {
                        if constexpr (std::is_scalar_v<U2>) {
                            if (!this->x().contains(a.x())) return false;
                            if (this->y() != a.y()) return false;
                            return true;
                        } else {
                            return false;
                        }
                    } else {
                        if (!this->x().contains(a.x())) return false;
                        if (!this->y().contains(a.y())) return false;
                        return true;
                    }
                }
            }
        }
    };
#pragma pack(pop)

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
    template <typename T1, typename T2, class Stream>
    auto operator<<(Stream& out, const point<T1, T2>& p) -> Stream& {
        out << "(" << p.x() << ", " << p.y() << ")";
        return out;
    }

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
