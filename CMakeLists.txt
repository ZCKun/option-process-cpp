cmake_minimum_required(VERSION 3.19)
project(option_process)
include(${CMAKE_SOURCE_DIR}/conan/conan_paths.cmake)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -ltbb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")
find_package(RdKafka)

file(GLOB HEADERS
        include/*.h
        include/flat_hash_map/*.hpp
        include/mysql-cppconn-8/*.h
        include/mysql-cppconn-8/jdbc/*.h
        include/mysql-cppconn-8/jdbc/cppconn/*.h
        include/mysql-cppconn-8/mysql/*.h
        include/mysql-cppconn-8/mysqlx/*.h
        include/mysql-cppconn-8/mysqlx/common/*.h
        include/mysql-cppconn-8/mysqlx/devapi/*.h
        include/mysql-cppconn-8/mysqlx/devapi/detail/*.h
        include/cppkafka/detail/*.h
        include/cppkafka/utils/*.h
        src/helper/*.h
        src/process.h
        )

file(GLOB SOURCE
        include/*.cpp
        src/helper/*.cpp
        src/process.cpp
        )

include_directories(include src)
link_directories(
        libs/mysqlconn
        libs/cppkafka
)

add_executable(option_process main.cpp ${HEADERS} ${SOURCE})
target_link_libraries(option_process
        mysqlcppconn8
#        RdKafka::rdkafka
#        cppkafka
        fmt
        )

#add_subdirectory(tests)
#add_subdirectory(tests/kafka)
# target_link_libraries(option_process PRIVATE GTest::gtest GTest::gtest_main)