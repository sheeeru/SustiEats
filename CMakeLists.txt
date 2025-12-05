# =========================
# CMakeLists.txt
# =========================

cmake_minimum_required(VERSION 3.10)
project(SustiEats VERSION 1.0 LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# Include the include/ folder
include_directories(${CMAKE_SOURCE_DIR}/include)

# Grab all .cpp source files in src/
file(GLOB SRC_FILES src/*.cpp)

# Build the executable
add_executable(SustiEats ${SRC_FILES} main.cpp)

# Link SFML libraries
target_link_libraries(
    SustiEats
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# Copy assets folder to build directory after build
add_custom_command(
    TARGET SustiEats POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:SustiEats>/assets
)
