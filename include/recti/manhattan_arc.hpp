#pragma once

// #include <algorithm> // import std::min, std::max
#include <utility>  // import std::move

#include "interval.hpp"
#include "point.hpp"

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief Merging Object (for deferred-merge embedding (DME) algorithm)
     *
     * The code is defining a class template called `ManhattanArc` that represents a
     * merging segment. The merging segment can include a single point, segment, or
     * region. The template has two type parameters `T1` and `T2`, which default to
     * `int` if not specified. The class inherits from the `Point` class, which
     * represents a point in a 2D coordinate system. The `ManhattanArc` class provides
     * various operations and functions for manipulating and comparing merging
     * segments, such as adding and subtracting vectors, checking for overlap and
     * intersection with other merging segments, calculating the minimum distance
     * between merging segments, and merging two merging segments. The class also
     * provides comparison operators and a stream insertion operator for convenient
     * usage. The purpose of the `ManhattanArc` class is to support the deferred-merge
     * embedding (DME) algorithm, as referenced in the code comments.
     *
     * Reference:
     *  - Ting-Hai Chao, Yu-Chin Hsu, Jan-Ming Ho and A. B. Kahng, "Zero skew clock
     * routing with minimum wirelength," in IEEE Transactions on Circuits and
     * Systems II: Analog and Digital Signal Processing, vol. 39, no. 11, pp.
     * 799-814, Nov. 1992, doi: 10.1109/82.204128.
     *
     * @tparam T1 int or Interval
     * @tparam T2 int or Interval
     */
    template <typename T1, typename T2 = T1> class ManhattanArc {
      public:
        Point<T1, T2> impl;

      public:
        /**
         * @brief Construct a new ManhattanArc object
         *
         * This code defines a constructor for the `ManhattanArc` class template. The
         * constructor takes two parameters `xcoord` and `ycoord`, which are of type
         * `T1` and `T2` respectively. The constructor is marked as `constexpr` and
         * `noexcept`, indicating that it can be evaluated at compile-time and it does
         * not throw any exceptions.
         *
         * @param[in] xcoord The x coordinate value.
         * @param[in] ycoord The y coordinate value.
         */
        constexpr ManhattanArc(T1 xcoord, T2 ycoord)
            : impl{std::move(xcoord), std::move(ycoord)} {}

        /**
         * @brief Construct a ManhattanArc from a Point.
         *
         * @param[in] pt The input point.
         * @return A new ManhattanArc.
         */
        static constexpr auto from_point(const Point<T1, T2>& pt) {
            auto pt_xformed = pt.rotates();
            return ManhattanArc<T1, T2>{pt_xformed.xcoord(), pt_xformed.ycoord()};
        }

        /**
         * @brief Construct a new ManhattanArc object from the given x and y coordinates.
         *
         * This static member function constructs a new `ManhattanArc` object by combining the
         * given `xcoord` and `ycoord` parameters in a specific way. The resulting `ManhattanArc`
         * object represents a merging segment that can include a single point, segment, or
         * region.
         *
         * @param[in] xcoord The x coordinate value.
         * @param[in] ycoord The y coordinate value.
         * @return A new `ManhattanArc` object constructed from the given coordinates.
         */
        static constexpr auto construct(T1&& xcoord, T2&& ycoord) -> ManhattanArc {
            return ManhattanArc{xcoord - ycoord, xcoord + ycoord};
        }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * @brief Compares two `ManhattanArc` objects for equality.
         *
         * This operator overload compares two `ManhattanArc` objects for equality. It returns `true` if
         * the underlying `Point<T1, T2>` objects are equal, and `false` otherwise.
         *
         * @tparam U1 The type of the x-coordinate of the right-hand side `ManhattanArc`.
         * @tparam U2 The type of the y-coordinate of the right-hand side `ManhattanArc`.
         * @param[in] rhs The `ManhattanArc` object to compare against.
         * @return `true` if the two `ManhattanArc` objects are equal, `false` otherwise.
         */
        template <typename U1, typename U2>
        constexpr auto operator==(const ManhattanArc<U1, U2>& rhs) const -> bool {
            return this->impl == rhs.impl;
        }

        /**
         * @brief Not equal to
         *
         * Compares two `ManhattanArc` objects for inequality.
         *
         * This operator overload compares two `ManhattanArc` objects for inequality. It returns `true`
         * if the underlying `Point<T1, T2>` objects are not equal, and `false` otherwise.
         *
         * @tparam U1 The type of the x-coordinate of the right-hand side `ManhattanArc`.
         * @tparam U2 The type of the y-coordinate of the right-hand side `ManhattanArc`.
         * @param[in] rhs The `ManhattanArc` object to compare against.
         * @return `true` if the two `ManhattanArc` objects are not equal, `false` otherwise.
         */
        template <typename U1, typename U2>
        constexpr auto operator!=(const ManhattanArc<U1, U2>& rhs) const -> bool {
            return this->impl != rhs.impl;
        }

        /**
         * @brief Check if two `ManhattanArc` objects overlap.
         *
         * This function checks if the x and y coordinates of the current `ManhattanArc` object overlap
         * with the x and y coordinates of the provided `ManhattanArc` object `other`. The overlap is
         * determined by checking if the ranges of the x and y coordinates intersect.
         *
         * @tparam U1 The type of the x and y coordinates of the current `ManhattanArc` object.
         * @tparam U2 The type of the x and y coordinates of the `ManhattanArc` object `other`.
         * @param other The `ManhattanArc` object to check for overlap.
         * @return `true` if the two `ManhattanArc` objects overlap, `false` otherwise.
         */
        template <typename U1, typename U2>  //
        constexpr auto overlaps(const ManhattanArc<U1, U2>& other) const -> bool {
            return this->impl.overlaps(other.impl);
        }

        /**
         * @brief Compute the intersection of two `ManhattanArc` objects.
         *
         * This function computes the intersection of the current `ManhattanArc` object with the
         * provided `ManhattanArc` object `other`. The intersection is calculated by finding the
         * intersection of the x and y coordinate ranges of the two `ManhattanArc` objects. The
         * resulting `ManhattanArc` object represents the overlapping region between the two input
         * `ManhattanArc` objects.
         *
         * @tparam U1 The type of the x and y coordinates of the current `ManhattanArc` object.
         * @tparam U2 The type of the x and y coordinates of the `ManhattanArc` object `other`.
         * @param[in] other The `ManhattanArc` object to intersect with the current `ManhattanArc` object.
         * @return A new `ManhattanArc` object representing the intersection of the two input `ManhattanArc`
         * objects.
         */
        template <typename U1, typename U2>  //
        constexpr auto intersect_with(const ManhattanArc<U1, U2>& other) const {
            auto xcoord = intersection(this->impl.xcoord(), other.impl.xcoord());
            auto ycoord = intersection(this->impl.ycoord(), other.impl.ycoord());
            return ManhattanArc<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                                std::move(ycoord)};
        }

        /**
         * @brief Compute the minimum distance between the x and y coordinates of two `ManhattanArc`
         * objects.
         *
         * This function calculates the maximum distance between the x and y coordinates of the
         * current `ManhattanArc` object and the provided `ManhattanArc` object `other`. The distance is
         * computed by taking the maximum of the distances between the x coordinates and the
         * distances between the y coordinates of the two `ManhattanArc` objects.
         *
         * @tparam U1 The type of the x and y coordinates of the current `ManhattanArc` object.
         * @tparam U2 The type of the x and y coordinates of the `ManhattanArc` object `other`.
         * @param[in] other The `ManhattanArc` object to compute the maximum distance with.
         * @return The maximum distance between the two `ManhattanArc` objects.
         */
        template <typename U1, typename U2>  //
        constexpr auto min_dist_with(const ManhattanArc<U1, U2>& other) const {
            return std::max(min_dist(this->impl.xcoord(), other.impl.xcoord()),
                            min_dist(this->impl.ycoord(), other.impl.ycoord()));
        }

        /**
         * @brief Enlarge the `ManhattanArc` object by a given scale factor.
         *
         * This function creates a new `ManhattanArc` object by enlarging the x and y coordinates of the
         * current `ManhattanArc` object by the given scale factor `alpha`. The resulting `ManhattanArc`
         * object will have larger x and y coordinate ranges.
         *
         * @tparam R The type of the scale factor `alpha`.
         * @param[in] alpha The scale factor to enlarge the `ManhattanArc` object by.
         * @return A new `ManhattanArc` object with enlarged x and y coordinates.
         */
        template <typename R>  //
        constexpr auto enlarge_with(const R& alpha) const {
            auto xcoord = enlarge(this->impl.xcoord(), alpha);
            auto ycoord = enlarge(this->impl.ycoord(), alpha);
            return ManhattanArc<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                                std::move(ycoord)};
        }

        /**
         * @brief Calculate the center of the ManhattanArc.
         *
         * @return The center as a Point.
         */
        constexpr auto get_center() const {
            auto m = impl.get_center();
            return m.inv_rotates();
        }

        /**
         * @brief Calculate the lower corner of the ManhattanArc.
         *
         * @return The lower corner as a Point.
         */
        constexpr auto get_lower_corner() const {
            auto m = impl.lower_corner();
            return m.inv_rotates();
        }

        /**
         * @brief Calculate the upper corner of the ManhattanArc.
         *
         * @return The upper corner as a Point.
         */
        constexpr auto get_upper_corner() const {
            auto m = impl.upper_corner();
            return m.inv_rotates();
        }

        /**
         * @brief Find the nearest point to a given Point.
         *
         * @param[in] other The reference Point.
         * @return The nearest Point on this ManhattanArc.
         */
        template <typename U1, typename U2>
        constexpr auto nearest_point_to(const Point<U1, U2>& other) const {
            auto ms = ManhattanArc<U1, U2>::from_point(other);
            auto distance = this->min_dist_with(ms);
            auto trr = ms.enlarge_with(distance);
            auto lb = impl.lower_corner();
            auto ub = impl.upper_corner();
            auto m = impl.get_center();
            if (trr.impl.contains(lb)) {
                m = lb;
            } else if (trr.impl.contains(ub)) {
                m = ub;
            }
            return m.inv_rotates();
        }

        /**
         * @brief Merge with another ManhattanArc using a given alpha.
         *
         * @param[in] other The other ManhattanArc.
         * @param[in] alpha The enlargement factor.
         * @return The merged ManhattanArc.
         */
        template <typename U1, typename U2>
        constexpr auto merge_with(const ManhattanArc<U1, U2>& other, int alpha) const {
            auto distance = min_dist_with(other);
            auto trr1 = this->enlarge_with(alpha);
            auto trr2 = other.enlarge_with(distance - alpha);
            auto localimpl = trr1.impl.intersect_with(trr2.impl);
            auto x = localimpl.xcoord();
            auto y = localimpl.ycoord();
            return ManhattanArc<decltype(x), decltype(y)>{std::move(x), std::move(y)};
        }
        
        /**
         * @brief Overload the stream insertion operator `<<` to output a `ManhattanArc` object.
         *
         * This function overloads the stream insertion operator `<<` to output a `ManhattanArc` object
         * in the format `"/{xcoord}, {ycoord}/"`, where `{xcoord}` and `{ycoord}` are the x and y
         * coordinates of the `ManhattanArc` object, respectively.
         *
         * @tparam Stream The type of the output stream.
         * @param[out] out The output stream to write the `ManhattanArc` object to.
         * @param[in] manhattan_arc The `ManhattanArc` object to be written to the output stream.
         * @return The modified output stream.
         */
        template <class Stream> friend auto operator<<(Stream& out, const ManhattanArc& manhattan_arc)
            -> Stream& {
            out << "/" << manhattan_arc.impl.xcoord() << ", " << manhattan_arc.impl.ycoord() << "/";
            return out;
        }

        // /**
        //  * @brief minimum distance with
        //  *
        //  * @param[in] other
        //  * @return constexpr auto
        //  */
        // constexpr auto min_dist_change_with(ManhattanArc& other) {
        //     auto minDist = this->min_dist_with(other);
        //     auto mobj1 = this->enlarge_with(minDist);
        //     auto mobj2 = other.enlarge_with(minDist);
        //     other = mobj1.intersect_with(other);
        //     *this = mobj2.intersect_with(*this);
        //     return minDist;
        // }
    };
#pragma pack(pop)

    // template <typename U1, typename U2>  //
    // constexpr auto min_dist_change_merge(U1& lhs, U2& rhs) {
    //     if constexpr (std::is_scalar<U1>::value) {
    //         if constexpr (std::is_scalar<U2>::value) {
    //             return std::abs(lhs - rhs);
    //         } else {
    //             return rhs.min_dist_change_with(lhs);
    //         }
    //     } else {
    //         return lhs.min_dist_change_with(rhs);
    //     }
    // }

}  // namespace recti
