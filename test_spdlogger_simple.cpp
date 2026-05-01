#include <fstream>
#include <iostream>
#include <recti/logger.hpp>

int main() {
    std::cout << "=== Simple Spdlogger Test ===\n";

    // Test the wrapper function
    std::cout << "Testing recti::log_with_spdlog()...\n";
    recti::log_with_spdlog("Simple test message 1");
    recti::log_with_spdlog("Simple test message 2");
    recti::log_with_spdlog("Simple test message 3");

    std::cout << "Messages logged to recti.log\n";

    // Verify the log file exists
    std::ifstream log_file("recti.log");
    if (log_file.is_open()) {
        std::cout << "✓ Log file exists and is readable.\n";
        std::string line;
        int line_count = 0;
        std::cout << "\nLog file contents:\n";
        std::cout << "---\n";
        while (std::getline(log_file, line)) {
            line_count++;
            std::cout << line << '\n';
        }
        log_file.close();
        std::cout << "---\n";
        std::cout << "Total lines: " << line_count << '\n';

        if (line_count >= 3) {
            std::cout << "\n✓ Test PASSED: All messages were logged successfully!\n";
        } else {
            std::cout << "\n✗ Test FAILED: Expected at least 3 lines, got " << line_count << '\n';
            return 1;
        }
    } else {
        std::cerr << "✗ Test FAILED: Could not open recti.log\n";
        return 1;
    }

    std::cout << "\n=== Test completed successfully ===\n";
    return 0;
}