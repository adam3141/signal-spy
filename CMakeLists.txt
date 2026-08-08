cmake_minimum_required(VERSION 3.30)
project(signal-spy VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The Release with Debug Info preset will also enable profiling with Tracy
if(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    add_definitions(-DTRACY_ENABLE)
endif()

# Create a link in the top level directory for the compilation database. The real database is located in the binary
# directory so we will need to create a symlink in the root workspace directory so that clangd can use the correct 
# version.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  file(CREATE_LINK
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json"
    SYMBOLIC COPYONERROR
  )
endif()

add_subdirectory(submodules)
