#pragma once

#include "interval.hpp"  // for interval
#include "point.hpp"     // for Point

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief Rectangle (Rectilinear)
     *
     * @tparam T
     */
    template <typename T> struct Rect : Point<interval<T>> {
        /**
         * @brief Construct a new Rect object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr Rect(interval<T>&& x, interval<T>&& y) noexcept
            : Point<interval<T>>{std::move(x), std::move(y)} {}

        /**
         * @brief Construct a new Rect object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr Rect(const interval<T>& x, const interval<T>& y) : Point<interval<T>>{x, y} {}

        /**
         * @brief
         *
         * @return Point<T>
         */
        [[nodiscard]] constexpr auto lb() const -> Point<T> {
            return {this->x().lb(), this->y().lb()};
        }

        /**
         * @brief
         *
         * @return Point<T>
         */
        [[nodiscard]] constexpr auto ub() const -> Point<T> {
            return {this->x().ub(), this->y().ub()};
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
    template <typename T> struct HSegment : Point<interval<T>, T> {
        /**
         * @brief Construct a new HSegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr HSegment(interval<T>&& x, T&& y) noexcept
            : Point<interval<T>, T>{std::move(x), std::move(y)} {}

        /**
         * @brief Construct a new HSegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr HSegment(const interval<T>& x, const T& y) : Point<interval<T>, T>{x, y} {}
    };
#pragma pack(pop)

#pragma pack(push, 1)
    /**
     * @brief VSegment Line Segment
     *
     * @tparam T
     */
    template <typename T> struct VSegment : Point<T, interval<T>> {
        /**
         * @brief Construct a new VSegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr VSegment(T&& x, interval<T>&& y) noexcept
            : Point<T, interval<T>>{std::move(x), std::move(y)} {}

        /**
         * @brief Construct a new VSegment object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr VSegment(const T& x, const interval<T>& y) : Point<T, interval<T>>{x, y} {}
    };
#pragma pack(pop)

}  // namespace recti
