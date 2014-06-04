project(ihamster)
cmake_minimum_required(VERSION 2.8)

SET( EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}" )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fsanitize=address" )
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -ggdb" )
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

find_package(PNG REQUIRED)

include_directories(${PNG_INCLUDE_DIR})
include_directories("../include")

set(IHAMSTER_CPP_FILES 
	adapter/config.cpp
	adapter/filesystem.cpp
	adapter/image.cpp
	adapter/plotcirc.cpp
	adapter/sqlite.cpp
	alg/paint.cpp
	alg/plotcirc_db.cpp
	alg/seg_object.cpp
	alg/sobel.cpp	
	alg/watershed.cpp	
	app/main.cpp
	cmn/log.cpp
	cmn/image.cpp
	cmn/image_utils.cpp
	cmn/plotcirc.cpp
)

add_executable( ihamster ${IHAMSTER_CPP_FILES} )
target_link_libraries( ihamster ${PNG_LIBRARIES} -lsqlite3 )

add_custom_command( TARGET ihamster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/../pictures"
         ${EXECUTABLE_OUTPUT_PATH}/pictures )