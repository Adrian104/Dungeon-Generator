message(">>> Processing CMakeLists.txt in dgen directory...")
project(dgen LANGUAGES CXX)

file(GLOB_RECURSE DGEN_CPP CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE DGEN_HPP CONFIGURE_DEPENDS "src/*.hpp")

add_library(${PROJECT_NAME} STATIC ${DGEN_CPP} ${DGEN_HPP})
target_include_directories(${PROJECT_NAME} PUBLIC "include")