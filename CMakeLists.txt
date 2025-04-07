cmake_minimum_required(VERSION 3.10)

project(IP-testi)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(IP-testi main.cpp)

target_link_libraries(IP-testi ws2_32)
