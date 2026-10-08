#define DOCTEST_CONFIG_IMPLEMENT

#include <doctest/doctest.h>

#include <filesystem>

int main(int argc, char** argv) {
    const std::filesystem::path output_dir
        = std::filesystem::temp_directory_path() / "recti_test_output";
    std::error_code ec;
    std::filesystem::create_directories(output_dir, ec);
    std::filesystem::current_path(output_dir, ec);

    doctest::Context context(argc, argv);
    return context.run();
}
