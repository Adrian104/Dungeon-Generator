message(">>> Processing CMakeLists.txt in dgen-app directory...")
project(dgen-app LANGUAGES CXX)
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
set(SDL2_TTF_INSTALL OFF)
option(SDL2_DISABLE_INSTALL "" OFF)
option(SDL2TTF_VENDORED "Use vendored third-party libraries" ON)

FetchContent_Declare(
	SDL2
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG release-2.30.4
	GIT_SHALLOW ON
)

FetchContent_Declare(
	SDL2_ttf
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
	GIT_TAG release-2.22.0
	GIT_SHALLOW ON
)

FetchContent_MakeAvailable(SDL2)
FetchContent_MakeAvailable(SDL2_ttf)

file(MAKE_DIRECTORY "${sdl2_ttf_SOURCE_DIR}/include/SDL2")
file(COPY "${sdl2_ttf_SOURCE_DIR}/SDL_ttf.h" DESTINATION "${sdl2_ttf_SOURCE_DIR}/include/SDL2")
set(SDL_TTF_INCLUDE_FIX "${sdl2_ttf_SOURCE_DIR}/include")

file(GLOB_RECURSE DGEN_APP_CPP CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE DGEN_APP_HPP CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE DGEN_APP_RES CONFIGURE_DEPENDS "res/*.*")

add_executable(${PROJECT_NAME} ${DGEN_APP_CPP} ${DGEN_APP_HPP} ${DGEN_APP_RES})
target_include_directories(${PROJECT_NAME} PRIVATE ${dgen_SOURCE_DIR}/src ${SDL_TTF_INCLUDE_FIX})
target_link_libraries(${PROJECT_NAME} PRIVATE dgen SDL2 SDL2main SDL2_ttf)