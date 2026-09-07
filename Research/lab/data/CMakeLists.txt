cmake_minimum_required(VERSION 3.16)

project(alpaca_collector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(
    alpaca_collector

    src/main.cpp
    src/alpaca.cpp
)

target_include_directories(
    alpaca_collector
    PRIVATE

    include
)

target_link_libraries(
    alpaca_collector

    PRIVATE
    CURL::libcurl
)
