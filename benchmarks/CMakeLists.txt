include(FetchContent)

message(STATUS "Fetching nanobench v4.3.11 ...")
FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG        v4.3.11
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nanobench)

add_executable(bench_ez_cli bench_ez_cli.cpp)
target_link_libraries(bench_ez_cli PRIVATE ezcli nanobench)
