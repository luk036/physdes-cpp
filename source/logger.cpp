#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <recti/logger.hpp>

namespace recti {

    void log_with_spdlog(const std::string& message) {
        // Always create a fresh logger to ensure proper file handling
        std::shared_ptr<spdlog::logger> logger;
        try {
            // Try to drop the existing logger first
            spdlog::drop("file_logger");
        } catch (...) {
            // Ignore if logger doesn't exist
        }

        // Create a new logger
        logger = spdlog::basic_logger_mt("file_logger", "recti.log");
        if (logger) {
            logger->set_level(spdlog::level::info);
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
            logger->flush_on(spdlog::level::info);
            logger->info("Recti message: {}", message);
            logger->flush();
        }
    }

}  // namespace recti