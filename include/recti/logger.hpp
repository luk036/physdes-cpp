/** @file logger.hpp
 *  @brief Logger wrapper for spdlog.
 */

#pragma once

#include <string>

namespace recti {

    /**
     * @brief Log a message using spdlog
     *
     * This function provides a simple wrapper around spdlog for logging messages.
     * It creates a file logger that writes to "recti.log" and logs at the info level.
     *
     * @param message The message to log
     */
    void log_with_spdlog(const std::string& message);

    /**
     * @brief Log a message at debug level using spdlog
     *
     * Identical to log_with_spdlog but emitted at debug level. The default
     * logger is configured at info level, so these messages are filtered out
     * and incur no formatting or file-flush cost in hot paths.
     *
     * @param message The message to log
     */
    void log_with_spdlog_debug(const std::string& message);

}  // namespace recti
