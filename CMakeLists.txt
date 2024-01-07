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

if(${BUILD_DGEN_APP})
	include(FetchContent)

	set(FETCHCONTENT_QUIET OFF)
	set(SDL2_TTF_INSTALL OFF)
	option(SDL2_DISABLE_INSTALL "" OFF)

	FetchContent_Declare(
		SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG release-2.28.5
		GIT_SHALLOW ON
	)

	FetchContent_Declare(
		SDL2_ttf
		GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
		GIT_TAG release-2.20.2
		GIT_SHALLOW ON
	)

	FetchContent_MakeAvailable(SDL2)
	FetchContent_MakeAvailable(SDL2_ttf)

	file(MAKE_DIRECTORY "${sdl2_ttf_SOURCE_DIR}/include/SDL2")
	file(COPY "${sdl2_ttf_SOURCE_DIR}/SDL_ttf.h" DESTINATION "${sdl2_ttf_SOURCE_DIR}/include/SDL2")
	set(SDL_TTF_INCLUDE_FIX "${sdl2_ttf_SOURCE_DIR}/include")
endif()

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