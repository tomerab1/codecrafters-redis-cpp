cmake_minimum_required(VERSION 3.13)
project(redis-starter-cpp)
set(CMAKE_CXX_STANDARD 20) # Enable the C++20 standard

# Include directories
include_directories(src)

# Source files
set(SOURCE_FILES 
    src/Server.cpp 
)

# Executable
add_executable(server ${SOURCE_FILES})

# Link libraries
target_link_libraries(server pthread)
