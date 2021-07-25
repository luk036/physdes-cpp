#include <ostream>
#include <recti/recti.hpp>

namespace recti {

    /**
     * @brief
     *
     * @tparam T
     * @tparam Stream
     * @param[out] out
     * @param[in] p
     * @return Stream&
     */
    template <typename T, class Stream> auto operator<<(Stream& out, const vector2<T>& v)
        -> Stream& {
        out << "{" << v.x() << ", " << v.y() << "}";
        return out;
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
    template <typename T1, typename T2, class Stream>
    auto operator<<(Stream& out, const point<T1, T2>& p) -> Stream& {
        out << "(" << p.x() << ", " << p.y() << ")";
        return out;
    }

    /**
     * @brief
     *
     * @tparam T
     * @tparam Stream
     * @param[out] out
     * @param[in] p
     * @return Stream&
     */
    template <typename T, class Stream> auto operator<<(Stream& out, const interval<T>& I)
        -> Stream& {
        out << "[" << I.lower() << ", " << I.upper() << "]";
        return out;
    }

    template auto operator<<(std::ostream& out, const vector2<int>& v) -> std::ostream&;
    template auto operator<<(std::ostream& out, const point<int, int>& p) -> std::ostream&;
    template auto operator<<(std::ostream& out, const interval<int>& p) -> std::ostream&;
    template auto operator<<(std::ostream& out, const point<interval<int>, interval<int>>& p)
        -> std::ostream&;
    template auto operator<<(std::ostream& out, const point<interval<int>, int>& p)
        -> std::ostream&;
    template auto operator<<(std::ostream& out, const point<int, interval<int>>& p)
        -> std::ostream&;

}  // namespace recti
