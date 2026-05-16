add_executable(test_ez_cli test_ez_cli.cpp)
target_link_libraries(test_ez_cli PRIVATE ezcli)
target_include_directories(test_ez_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME ezcli_tests COMMAND test_ez_cli)
