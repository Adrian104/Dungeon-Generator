project(dgen-app LANGUAGES CXX)
include(FetchContent)
find_package(SDL3 QUIET)

if(SDL3_FOUND)
	message(STATUS "${PROJECT_NAME}: Library 'SDL3' is installed, no need to download.")
else()
	message(STATUS "${PROJECT_NAME}: Library 'SDL3' is not installed, downloading...")
	FetchContent_Declare(
		SDL3
		GIT_REPOSITORY https://github.com/libsdl-org/SDL
		GIT_TAG release-3.4.16
		GIT_SHALLOW ON
	)

	FetchContent_MakeAvailable(SDL3)
endif()

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.92.9b
	GIT_SHALLOW ON
)

message(STATUS "${PROJECT_NAME}: Downloading 'imgui' library...")
FetchContent_MakeAvailable(imgui)

add_library(
	imgui
	"${imgui_SOURCE_DIR}/imgui.cpp"
	"${imgui_SOURCE_DIR}/imgui_demo.cpp"
	"${imgui_SOURCE_DIR}/imgui_draw.cpp"
	"${imgui_SOURCE_DIR}/imgui_tables.cpp"
	"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
)

target_include_directories(
	imgui
	PUBLIC
	"${imgui_SOURCE_DIR}"
	"${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)

file(GLOB_RECURSE DGEN_APP_CPP CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE DGEN_APP_HPP CONFIGURE_DEPENDS "src/*.hpp")

add_executable(${PROJECT_NAME} ${DGEN_APP_CPP} ${DGEN_APP_HPP})

target_link_libraries(${PROJECT_NAME} PRIVATE dgen SDL3::SDL3 imgui)
target_include_directories(${PROJECT_NAME} PRIVATE "${dgen_SOURCE_DIR}/src")
target_precompile_headers(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/pch.hpp")
