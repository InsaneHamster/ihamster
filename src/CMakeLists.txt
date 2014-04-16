project(ihamster)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") # -fsanitize=address" )
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}") # -fsanitize=address")

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

include_directories("../include")

set(IHAMSTER_CPP_FILES 
	adapter/image.cpp
	alg/watershed.cpp
	app/main.cpp
	cmn/log.cpp
	cmn/image.cpp
)

add_executable( ihamster ${IHAMSTER_CPP_FILES} )
target_link_libraries( ihamster ${PNG_LIBRARIES})
