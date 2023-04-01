#pragma once

#include <cmath>
#include <vector>

namespace recti {

/**
 * @brief van der Corput sequence
 *
 * @param[in] k
 * @param[in] base
 * @param[in] scale
 * @return unsigned
 */
inline auto vdc(unsigned num, unsigned base = 2, unsigned scale = 10) noexcept
    -> unsigned {
  auto vdc = 0U;
  auto factor = unsigned(std::pow(base, scale));
  while (num != 0) {
    auto remainder = num % base;
    factor /= base;
    num /= base;
    vdc += remainder * factor;
  }
  return vdc;
}

/**
 * @brief van der Corput sequence generator
 *
 */
class Vdcorput {
private:
  unsigned _count{0};
  unsigned _base;
  unsigned _scale;

public:
  /**
   * @brief Construct a new Vdcorput object
   *
   * @param[in] base
   * @param[in] scale
   */
  constexpr explicit Vdcorput(unsigned base = 2, unsigned scale = 10) noexcept
      : _base{base}, _scale{scale} {}

  /**
   * @brief
   *
   * @return double
   */
  auto operator()() noexcept -> unsigned {
    this->_count += 1;
    return vdc(this->_count, this->_base, this->_scale);
  }

  /**
   * @brief
   *
   * @param[in] seed
   */
  auto reseed(unsigned seed) noexcept -> void { this->_count = seed; }
};

/**
 * @brief Halton sequence generator
 *
 */
class halton {
private:
  Vdcorput _vdc0;
  Vdcorput _vdc1;

public:
  /**
   * @brief Construct a new halton object
   *
   * @param[in] base
   * @param[in] scale
   */
  explicit constexpr halton(const unsigned base[],
                            const unsigned scale[]) noexcept
      : _vdc0(base[0], scale[0]), _vdc1(base[1], scale[1]) {}

  /**
   * @brief
   *
   * @return auto
   */
  auto operator()() -> std::vector<unsigned> {
    return {this->_vdc0(), this->_vdc1()};
  }

  /**
   * @brief
   *
   * @param[in] seed
   */
  auto reseed(unsigned seed) noexcept -> void {
    this->_vdc0.reseed(seed);
    this->_vdc1.reseed(seed);
  }
};

} // namespace recti
