cmake_minimum_required(VERSION 3.10)
project(NeonOubliette VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find SDL2
find_package(SDL2 REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/feature_flags.cpp
    src/stb_image.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Link SDL2
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
