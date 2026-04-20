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

enable_testing()

add_executable(pathogen_system_tests
    tests/pathogen_system_tests.cpp
)
target_include_directories(pathogen_system_tests PRIVATE src)
target_link_libraries(pathogen_system_tests PRIVATE SDL2::SDL2)
add_test(NAME pathogen_system_tests COMMAND pathogen_system_tests)

add_executable(survival_inventory_tests
    tests/survival_inventory_tests.cpp
)
target_include_directories(survival_inventory_tests PRIVATE src)
target_link_libraries(survival_inventory_tests PRIVATE SDL2::SDL2)
add_test(NAME survival_inventory_tests COMMAND survival_inventory_tests)

add_executable(simulation_alert_tests
    tests/simulation_alert_tests.cpp
)
target_include_directories(simulation_alert_tests PRIVATE src)
target_link_libraries(simulation_alert_tests PRIVATE SDL2::SDL2)
add_test(NAME simulation_alert_tests COMMAND simulation_alert_tests)

add_executable(visual_feedback_tests
    tests/visual_feedback_tests.cpp
)
target_include_directories(visual_feedback_tests PRIVATE src)
target_link_libraries(visual_feedback_tests PRIVATE SDL2::SDL2)
add_test(NAME visual_feedback_tests COMMAND visual_feedback_tests)

add_executable(equipment_tests
    tests/equipment_tests.cpp
)
target_include_directories(equipment_tests PRIVATE src)
target_link_libraries(equipment_tests PRIVATE SDL2::SDL2)
add_test(NAME equipment_tests COMMAND equipment_tests)

add_executable(target_selection_tests
    tests/target_selection_tests.cpp
)
target_include_directories(target_selection_tests PRIVATE src)
target_link_libraries(target_selection_tests PRIVATE SDL2::SDL2)
add_test(NAME target_selection_tests COMMAND target_selection_tests)

add_executable(discrete_inventory_tests
    tests/discrete_inventory_tests.cpp
)
target_include_directories(discrete_inventory_tests PRIVATE src)
target_link_libraries(discrete_inventory_tests PRIVATE SDL2::SDL2)
add_test(NAME discrete_inventory_tests COMMAND discrete_inventory_tests)

add_executable(market_trade_tests
    tests/market_trade_tests.cpp
)
target_include_directories(market_trade_tests PRIVATE src)
target_link_libraries(market_trade_tests PRIVATE SDL2::SDL2)
add_test(NAME market_trade_tests COMMAND market_trade_tests)

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets)
