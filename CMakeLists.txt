cmake_minimum_required(VERSION 3.10)
project(NeonOubliette VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED)

set(SOURCES
    src/main.cpp
    src/stb_image.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

enable_testing()

add_executable(world_generation_tests
    tests/world_generation_tests.cpp
)
target_include_directories(world_generation_tests PRIVATE src)
add_test(NAME world_generation_tests COMMAND world_generation_tests)

add_executable(world_builder_tests
    tests/world_builder_tests.cpp
)
target_include_directories(world_builder_tests PRIVATE src)
add_test(NAME world_builder_tests COMMAND world_builder_tests)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets)
