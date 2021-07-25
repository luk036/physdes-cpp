#pragma once

#include <utility>  // import std::move

#include "interval.hpp"
#include "point.hpp"

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief Rectangle (Rectilinear)
     *
     * @tparam T
     * @todo use "__attribute__((aligned(0)))" to align struct 'rectangle<int>' to 0
     * bytes
     */
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
         * @tparam U1
         * @tparam U2
         * @param rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        [[nodiscard]] constexpr auto overlaps(const point<U1, U2>& rhs) const -> bool {
            return this->x().overlaps(rhs.x()) && this->y().overlaps(rhs.y());
        }

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
    };
#pragma pack(pop)

#pragma pack(push, 1)
    /**
     * @brief Horizontal Line Segment
     *
     * @tparam T
     * @todo pack
     */
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
         * @tparam U1
         * @tparam U2
         * @param rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        [[nodiscard]] constexpr auto overlaps(const point<U1, U2>& rhs) const -> bool {
            return this->x().overlaps(rhs.x()) && rhs.y().overlaps(this->y());
        }

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

#pragma pack(push, 1)
    /**
     * @brief vsegment Line Segment
     *
     * @tparam T
     * @todo pack
     */
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
         * @tparam U1
         * @tparam U2
         * @param rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        [[nodiscard]] constexpr auto overlaps(const point<U1, U2>& rhs) const -> bool {
            return rhs.x().overlaps(this->x()) && this->y().overlaps(rhs.y());
        }

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
