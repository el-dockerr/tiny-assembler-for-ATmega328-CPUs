cmake_minimum_required(VERSION 3.10)
project(ATmega328Compiler)

# Set C++ standard 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(SOURCES
    src/main.cpp
    src/ATmega328Compiler.cpp
)

# Define header files
set(HEADERS
    src/ATmega328Compiler.hpp
    src/OpcodeMap.hpp
)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/src)

# Add executable with all sources
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add compiler flags
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

# Enable testing
enable_testing()
add_test(NAME ${PROJECT_NAME}Test 
         COMMAND ${PROJECT_NAME} --version)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)