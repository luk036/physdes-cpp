// #include <algorithm>
// #include <functional>
// #include <numeric> // import accumulate
// #include <recti/rpolygon.hpp>

// using namespace recti;

// /**
//  * @brief
//  *
//  * @tparam T
//  * @param pointset
//  * @return rpolygon<T>
//  */
// template <typename FwIter>
// static void create_xmonotone_i(FwIter&& first, FwIter&& last)
// {
//     auto l2r = [](const auto& a, const auto& b) { return a.x() < b.x(); };
//     auto r2l = [](const auto& a, const auto& b) { return b.x() < a.x(); };
//     auto [min, max] = std::minmax_element(first, last, l2r);
//     auto pivot = max->y();

//     using Fn = std::function<bool(const point<int>&)>;
//     Fn downup = [&pivot](const auto& a) { return a.y() < pivot; };
//     Fn updown = [&pivot](const auto& a) { return pivot < a.y(); };
//     auto middle =
//         std::partition(first, last, (min->y() < pivot) ? downup : updown);
//     std::sort(first, middle, std::move(l2r));
//     std::sort(middle, last, std::move(r2l));
// }

// /**
//  * @brief
//  *
//  * @tparam T
//  * @param pointset
//  * @return rpolygon<T>
//  */
// template <typename T>
// rpolygon<T> rpolygon<T>::create_xmonotone(std::vector<point<T>> pointset)
// {
//     create_xmonotone_i(pointset.begin(), pointset.end());
//     return {std::move(pointset)};
// }

// /**
//  * @brief
//  *
//  * @tparam T
//  * @param pointset
//  * @return rpolygon<T>
//  */
// template <typename T>
// rpolygon<T> rpolygon<T>::create_ymonotone(std::vector<point<T>> pointset)
// {
//     using D = std::vector<dualpoint<T>>; // x <-> y
//     auto first = reinterpret_cast<D&>(pointset).begin();
//     auto last = reinterpret_cast<D&>(pointset).end();
//     create_xmonotone_i(std::move(first), std::move(last));
//     return {std::move(pointset)};
// }


// /**
//  * @brief
//  *
//  * @tparam T
//  * @param pointset
//  * @return rpolygon<T>
//  */
// // template <typename FwIter>
// // static void create_regular_recur(FwIter&& first, FwIter&& last)
// // {
// //     if (first == last) return;

// //     auto l2r = [](const auto& a, const auto& b) { return a.x() < b.x(); };
// //     auto r2l = [](const auto& a, const auto& b) { return b.x() < a.x(); };
// //     auto [min, max] = std::minmax_element(first, last, l2r);
// //     auto pivot = max->y();

// //     using Fn = std::function<bool(const point<int>&)>;
// //     Fn downup = [&pivot](const auto& a) { return a.y() < pivot; };
// //     Fn updown = [&pivot](const auto& a) { return pivot < a.y(); };
// //     auto middle =
// //         std::partition(first, last, (min->y() < pivot) ? downup : updown);
// //     create_regular_recur(first, middle, std::move(l2r));
// //     create_regular_recur(middle, last, std::move(r2l));
// // }

// /**
//  * @brief
//  *
//  * @tparam T
//  * @param pointset
//  * @return rpolygon<T>
//  */
// // template <typename FwIter>
// // static void create_regular_i(FwIter&& first, FwIter&& last)
// // {
// //     auto l2r = [](const auto& a, const auto& b) { return a.x() < b.x(); };
// //     auto r2l = [](const auto& a, const auto& b) { return b.x() < a.x(); };
// //     auto [min, max] = std::minmax_element(first, last, l2r);
// //     auto pivot = max->y();

// //     using Fn = std::function<bool(const point<int>&)>;
// //     Fn downup = [&pivot](const auto& a) { return a.y() < pivot; };
// //     Fn updown = [&pivot](const auto& a) { return pivot < a.y(); };
// //     auto middle =
// //         std::partition(first, last, (min->y() < pivot) ? downup : updown);
// //     create_regular_recur(first, middle, std::move(l2r));
// //     create_regular_recur(middle, last, std::move(r2l));
// // }

// /**
//  * @brief
//  *
//  * @tparam T
//  * @param pointset
//  * @return rpolygon<T>
//  */
// // template <typename T>
// // rpolygon<T> rpolygon<T>::create_regular(std::vector<point<T>> pointset)
// // {
// //     using D = std::vector<dualpoint<T>>; // x <-> y
// //     auto first = reinterpret_cast<D&>(pointset).begin();
// //     auto last = reinterpret_cast<D&>(pointset).end();
// //     create_regular_i(std::move(first), std::move(last));
// //     return {std::move(pointset)};
// // }

// /**
//  * @brief area
//  *
//  * @tparam T
//  * @return T
//  */
// template <typename T>
// T rpolygon<T>::area() const
// {
//     auto it = this->begin();
//     const auto x0 = it->x();
//     auto yi = it->y();
//     auto sum = T {};
//     while (++it != this->end())
//     {
//         sum += (it->y() - yi) * (it->x() - x0);
//         yi = it->y();
//     }
//     return sum;
// }


// namespace recti
// {

// // Instantiation
// template class rpolygon<int>;

// } // namespace recti
