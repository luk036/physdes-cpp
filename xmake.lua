set_languages("c++20")

add_rules("mode.debug", "mode.release", "mode.coverage")
add_requires("fmt >= 12.1.0", "doctest", "nanobench")
add_requires("spdlog v1.17.0", {configs = {fmt_external = true}})

add_includedirs("include")

if is_mode("coverage") then
    add_cxflags("-ftest-coverage", "-fprofile-arcs", {force = true})
end

if is_mode("release") then
    if is_plat("windows") then
        add_cxflags("/GL", {force = true})
        add_ldflags("/LTCG", {force = true})
    elseif is_plat("linux") then
        add_cxflags("-flto", {force = true})
        add_ldflags("-flto", {force = true})
    end
end

if is_plat("linux") then
    set_warnings("all", "error")
    -- Termux/Android ships its libc++ headers under $PREFIX
    local prefix = os.getenv("PREFIX")
    if prefix then
        add_cxflags("-Wno-unused-command-line-argument", {force = true})
        add_sysincludedirs(prefix .. "/include/c++/v1", prefix .. "/include", {public = true})
    end
elseif is_plat("windows") then
    add_cxflags("/EHsc /utf-8 /W4 /WX /wd4702", {force = true})
    add_ldflags("/FORCE:MULTIPLE", {force = true})
end

target("Recti")
    set_kind("static")
    add_files("source/*.cpp")
    add_packages("fmt", "spdlog")

target("test_recti")
    add_deps("Recti")
    add_includedirs("../lds-cpp/include", {public = true})
    add_files("test/source/*.cpp")
    add_packages("doctest", "fmt", "spdlog")
    add_tests("default")

target("bench_dme")
    add_deps("Recti")
    add_files("bench/bench_dme.cpp")
    add_packages("nanobench", "fmt", "spdlog")

target("bench_main")
    add_deps("Recti")
    add_files("bench/bench_main.cpp")
    add_packages("nanobench", "fmt", "spdlog")

target("bench_full")
    add_deps("Recti")
    add_includedirs("experiments", {public = true})
    add_files("bench/bench_full.cpp")
    add_packages("fmt", "spdlog")

target("bench_robust")
    add_deps("Recti")
    add_files("bench/bench_robust.cpp")
    add_packages("fmt", "spdlog")

target("bench_repr")
    add_deps("Recti")
    add_files("bench/bench_repr.cpp")
    add_packages("fmt", "spdlog")

target("bench_cut")
    add_deps("Recti")
    add_files("bench/bench_cut.cpp")
    add_packages("fmt", "spdlog")

target("bench_router")
    add_deps("Recti")
    add_files("bench/bench_router.cpp")
    add_packages("fmt", "spdlog")

target("bench_steiner")
    add_files("bench/bench_steiner.cpp")

target("steiner_forest_grid")
    add_files("experiments/steiner_forest_grid.cpp")
