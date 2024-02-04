cmake_minimum_required(VERSION 3.22)
project(rlsudoku)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)



file(GLOB_RECURSE src_files
        "src/*.h"
        "src/*.cpp")

set(CMAKE_EXE_LINKER_FLAGS "-static")

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

add_subdirectory("vendor/raylib")
add_subdirectory("vendor/json")
add_subdirectory("vendor/jczsolve")

add_executable(rlsudoku ${src_files})

target_precompile_headers(rlsudoku PRIVATE src/pch.cxx src/pch.hxx)
target_link_libraries(rlsudoku PUBLIC raylib jczsolve nlohmann_json::nlohmann_json -static-libgcc -static-libstdc++)
target_include_directories(rlsudoku PUBLIC "vendor" PUBLIC "src")
set_target_properties(rlsudoku PROPERTIES LINKER_LANGUAGE CXX)
