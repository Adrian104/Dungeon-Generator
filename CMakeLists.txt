cmake_minimum_required(VERSION 3.25)
project(Dungeon-Generator LANGUAGES CXX)

option(BUILD_DGEN_APP "Build dgen-app project" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_DGEN_BENCHMARK "Build dgen-benchmark project" ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(dgen)

if(${BUILD_DGEN_APP})
	add_subdirectory(dgen-app)
endif()

if(${BUILD_DGEN_BENCHMARK})
	add_subdirectory(dgen-benchmark)
endif()
