cmake_minimum_required(VERSION 3.1)
project(cfx2_08)

set(library ${PROJECT_NAME})

file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/*.c
)

add_library(${library} ${sources})

target_include_directories(${PROJECT_NAME} PUBLIC
    include
)
