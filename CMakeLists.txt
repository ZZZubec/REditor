cmake_minimum_required (VERSION 3.4.10)

# Name your project. Can be anything.
project (REditor)

# Minimal supported C++ version.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(URHO3D_SAMPLES ON)

# Set binary output directories. Changing these paths will require you adjusting your resource path configuration!
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include cloned engine directory in a build.
add_subdirectory(rbfx)

file(GLOB Dir1_Sources RELATIVE "Sources" "*.cpp")

# Define executable name.
add_executable(REditor WIN32 Sources/REApplication.h Sources/REApplication.cpp ${Dir1_Sources})

# Link to game engine library.
target_link_libraries(REditor Urho3D)