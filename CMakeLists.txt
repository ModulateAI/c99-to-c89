cmake_minimum_required(VERSION 3.22)

# Ensure we reference the CMAKE_TOOLCHAIN_FILE variable so we don't get a warning about it being unused when re-running
# CMake on the same build folder.
message(STATUS "vcpkg integration will expected to be configured by toolchain: ${CMAKE_TOOLCHAIN_FILE}")

project(C99_to_C89)

set(C99_SOURCE_MAIN_ROOT "src/main")

# Build steps for `c99conv`.
add_executable(c99conv ${C99_SOURCE_MAIN_ROOT}/convert.c)

# Clang is provided by the LLVM project, which gets installed by vcpkg.
find_package(Clang CONFIG REQUIRED)

target_include_directories(c99conv PRIVATE ${CLANG_INCLUDE_DIRS})

# Link against Clang library
target_link_libraries(c99conv PRIVATE libclang)

# Build steps for `c99wrap`.
add_executable(c99wrap ${C99_SOURCE_MAIN_ROOT}/compilewrap.c)
