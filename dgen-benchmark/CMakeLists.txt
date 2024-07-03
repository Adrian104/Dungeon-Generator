message(">>> Processing CMakeLists.txt in dgen-benchmark directory...")
project(dgen-benchmark LANGUAGES CXX)

file(GLOB_RECURSE DGEN_BENCHMARK_CPP CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE DGEN_BENCHMARK_HPP CONFIGURE_DEPENDS "src/*.hpp")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
file(COPY "config.ini" DESTINATION "${CMAKE_BINARY_DIR}/bin")

add_executable(${PROJECT_NAME} ${DGEN_BENCHMARK_CPP} ${DGEN_BENCHMARK_HPP})
target_include_directories(${PROJECT_NAME} PRIVATE ${dgen_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE dgen)