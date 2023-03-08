#pragma once

#include <string>

namespace recti {

/**  Language codes to be used with the Recti class */
enum class LanguageCode { EN, DE, ES, FR };

/**
 * @brief A class for saying hello in multiple languages
 */
class Recti {
  std::string name;

public:
  /**
   * @brief Creates a new recti
   * @param[in] name the name to greet
   */
  Recti(std::string name);

  /**
   * @brief Creates a localized string containing the greeting
   * @param[in] lang the language to greet in
   * @return a string containing the greeting
   */
  std::string greet(LanguageCode lang = LanguageCode::EN) const;
};

} // namespace recti
