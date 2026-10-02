cmake_minimum_required(VERSION 3.16)
project(DoorAccessControl VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find SQLite3
find_package(SQLite3 REQUIRED)

# Find OpenSSL for password hashing
find_package(OpenSSL REQUIRED)

# Collect all source files
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Main executable
add_executable(door_access ${SOURCES})

target_include_directories(door_access PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(door_access PRIVATE
    SQLite3::SQLite3
    OpenSSL::Crypto
)

# Install target
install(TARGETS door_access DESTINATION bin)

# Copy database schema to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/sql/schema.sql
    ${CMAKE_BINARY_DIR}/sql/schema.sql
    COPYONLY
)

configure_file(
    ${CMAKE_SOURCE_DIR}/sql/seed_data.sql
    ${CMAKE_BINARY_DIR}/sql/seed_data.sql
    COPYONLY
)
