add_executable(fuzz_ez_cli fuzz_ez_cli.cpp)
target_link_libraries(fuzz_ez_cli PRIVATE ezcli)

target_compile_options(fuzz_ez_cli PRIVATE -fsanitize=fuzzer)
target_link_options(fuzz_ez_cli    PRIVATE -fsanitize=fuzzer)
