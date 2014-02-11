project(ihamster)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("../include")

set(IHAMSTER_CPP_FILES app/main.cpp )

add_executable( ihamster ${IHAMSTER_CPP_FILES} )
