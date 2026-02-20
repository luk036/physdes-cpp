#include <recti/logger.hpp>
#include <iostream>
#include <fstream>

int main() {
    std::cout << "=== Simple Spdlogger Test ===" << std::endl;

    // Test the wrapper function
    std::cout << "Testing recti::log_with_spdlog()..." << std::endl;
    recti::log_with_spdlog("Simple test message 1");
    recti::log_with_spdlog("Simple test message 2");
    recti::log_with_spdlog("Simple test message 3");

    std::cout << "Messages logged to recti.log" << std::endl;

    // Verify the log file exists
    std::ifstream log_file("recti.log");
    if (log_file.is_open()) {
        std::cout << "✓ Log file exists and is readable." << std::endl;
        std::string line;
        int line_count = 0;
        std::cout << "\nLog file contents:" << std::endl;
        std::cout << "---" << std::endl;
        while (std::getline(log_file, line)) {
            line_count++;
            std::cout << line << std::endl;
        }
        log_file.close();
        std::cout << "---" << std::endl;
        std::cout << "Total lines: " << line_count << std::endl;

        if (line_count >= 3) {
            std::cout << "\n✓ Test PASSED: All messages were logged successfully!" << std::endl;
        } else {
            std::cout << "\n✗ Test FAILED: Expected at least 3 lines, got " << line_count << std::endl;
            return 1;
        }
    } else {
        std::cerr << "✗ Test FAILED: Could not open recti.log" << std::endl;
        return 1;
    }

    std::cout << "\n=== Test completed successfully ===" << std::endl;
    return 0;
}