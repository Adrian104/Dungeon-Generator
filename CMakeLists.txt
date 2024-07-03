cmake_minimum_required(VERSION 3.21)
message(">>> Processing CMakeLists.txt in Dungeon-Generator directory...")
project(Dungeon-Generator LANGUAGES CXX)

if(${PROJECT_IS_TOP_LEVEL})
	message(">>> Dungeon-Generator project is top level.")
	set(DGEN_DEFAULT_MODE ON)
else()
	message(">>> Dungeon-Generator project is NOT top level.")
	set(DGEN_DEFAULT_MODE OFF)
endif()

option(BUILD_DGEN_APP "Build dgen-app project" ${DGEN_DEFAULT_MODE})
option(BUILD_DGEN_BENCHMARK "Build dgen-benchmark project" ${DGEN_DEFAULT_MODE})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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