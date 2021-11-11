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
    };
#pragma pack(pop)

#pragma pack(push, 1)
    /**
     * @brief vsegment Line Segment
     *
     * @tparam T
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
    };
#pragma pack(pop)

}  // namespace recti
