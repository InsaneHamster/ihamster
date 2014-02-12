project(ihamster)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

include_directories("../include")

set(IHAMSTER_CPP_FILES 
	adapter/image.cpp
	app/main.cpp
	cmn/log.cpp)

add_executable( ihamster ${IHAMSTER_CPP_FILES} )
target_link_libraries( ihamster ${PNG_LIBRARIES})
