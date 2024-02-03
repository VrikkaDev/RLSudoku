project(JCZSolve)

set(CMAKE_C_STANDARD 99)

add_library(jczsolve STATIC JCZSolve.c JCZSolve.h)
target_include_directories(jczsolve PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})