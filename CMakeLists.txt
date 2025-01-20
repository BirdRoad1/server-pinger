cmake_minimum_required(VERSION 3.13)

# Set the project name
project(mcpinger)

# Specify the C++ standard (optional)
set(CMAKE_CXX_STANDARD 20)
set(SOURCE_FILES src/main.cpp src/database.cpp src/mcping.cpp src/scanner.cpp src/serverdata.cpp src/stats.cpp src/cidr.cpp src/data/packetbuffer.cpp src/exception/net_exception.cpp src/exception/ping_parse_exception.cpp src/progress_writer.cpp src/logger.cpp)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

# Add an executable target
add_executable(mcpinger ${SOURCE_FILES})
target_compile_options(mcpinger PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(mcpinger -lpqxx)