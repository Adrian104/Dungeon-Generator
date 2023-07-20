message(">>> Processing CMakeLists.txt in dgen-benchmark directory...")
project(dgen-benchmark LANGUAGES CXX)

file(GLOB_RECURSE DGEN_BENCHMARK_CPP CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE DGEN_BENCHMARK_HPP CONFIGURE_DEPENDS "src/*.hpp")

add_executable(dgen-benchmark ${DGEN_BENCHMARK_CPP} ${DGEN_BENCHMARK_HPP})
target_include_directories(dgen-benchmark PRIVATE ${CMAKE_SOURCE_DIR}/dgen/src)
target_link_libraries(dgen-benchmark PRIVATE dgen)