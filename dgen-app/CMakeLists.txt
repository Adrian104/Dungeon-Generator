message(">>> Processing CMakeLists.txt in dgen-app directory...")
project(dgen-app LANGUAGES CXX)

file(GLOB_RECURSE DGEN_APP_CPP CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE DGEN_APP_HPP CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE DGEN_APP_RES CONFIGURE_DEPENDS "res/*.*")

add_executable(${PROJECT_NAME} ${DGEN_APP_CPP} ${DGEN_APP_HPP} ${DGEN_APP_RES})
target_include_directories(${PROJECT_NAME} PRIVATE ${dgen_SOURCE_DIR}/src ${SDL_TTF_INCLUDE_FIX})
target_link_libraries(${PROJECT_NAME} PRIVATE dgen SDL2 SDL2main SDL2_ttf)