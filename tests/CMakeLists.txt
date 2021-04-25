project(tests)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

find_package(GTest)
add_executable(tests iv_test.cpp)
target_link_libraries(tests
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        )
