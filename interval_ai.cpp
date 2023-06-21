#include <iostream>
#include <type_traits>
#include <variant>

template<typename T>
class Interval {
public:
    Interval(T lb, T ub) : _lb(lb), _ub(ub) {}
    bool operator<(const T& other) const {
        return _ub < other;
    }
    bool operator>(const T& other) const {
        return _lb > other;
    }
    bool overlaps(const Interval<T>& other) const {
        return !(_ub < other._lb || other._ub < _lb);
    }
    bool contains(const T& obj) const {
        return _lb <= obj && obj <= _ub;
    }
    bool contains(const Interval<T>& obj) const {
        return _lb <= obj._lb && obj._ub <= _ub;
    }
private:
    T _lb;
    T _ub;
};

template<typename T>
bool overlap(const T& lhs, const T& rhs) {
    if constexpr (std::is_member_function_pointer_v<decltype(&T::overlaps)>) {
        return lhs.overlaps(rhs);
    } else if constexpr (std::is_member_function_pointer_v<decltype(&T::overlaps)>) {
        return rhs.overlaps(lhs);
    } else {
        return lhs == rhs;
    }
}

template<typename T>
bool contain(const T& lhs, const T& rhs) {
    if constexpr (std::is_member_function_pointer_v<decltype(&T::contains)>) {
        return lhs.contains(rhs);
    } else if constexpr (std::is_member_function_pointer_v<decltype(&T::contains)>) {
        return false;
    } else {
        return lhs == rhs;
    }
}

int main() {
    Interval<int> a(4, 8);
    Interval<int> b(5, 6);
    std::cout << std::boolalpha;
    // std::cout << !(a < b) << std::endl;
    // std::cout << !(b < a) << std::endl;
    std::cout << a.contains(4) << std::endl;
    std::cout << a.contains(8) << std::endl;
    std::cout << a.contains(b) << std::endl;
    std::cout << a.overlaps(b) << std::endl;
    // std::cout << a.overlaps(6) << std::endl;
    return 0;
}

