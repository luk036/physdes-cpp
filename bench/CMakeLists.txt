add_executable(benchmark_recti bench_main.cpp)
target_link_libraries(benchmark_recti PRIVATE Recti benchmark::benchmark benchmark::benchmark_main)
