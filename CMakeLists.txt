cmake_minimum_required(VERSION 3.21)
message(">>> Processing CMakeLists.txt in Dungeon-Generator directory...")
project(Dungeon-Generator LANGUAGES CXX)

if(${PROJECT_IS_TOP_LEVEL})
	message(">>> Project is top level, all targets will be generated")
else()
	message(">>> Project is NOT top level, only dgen target will be generated")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(${PROJECT_IS_TOP_LEVEL})
	include(FetchContent)

	set(FETCHCONTENT_QUIET OFF)
	set(SDL2_TTF_INSTALL OFF)
	option(SDL2_DISABLE_INSTALL "" OFF)

	FetchContent_Declare(
		SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG release-2.28.5
		GIT_SHALLOW ON
		GIT_PROGRESS ON
	)

	FetchContent_Declare(
		SDL2_ttf
		GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
		GIT_TAG release-2.20.2
		GIT_SHALLOW ON
		GIT_PROGRESS ON
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

if(${PROJECT_IS_TOP_LEVEL})
	add_subdirectory(dgen-app)
	add_subdirectory(dgen-benchmark)
endif()