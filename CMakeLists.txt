cmake_minimum_required(VERSION 3.22)

# Ensure we reference the CMAKE_TOOLCHAIN_FILE variable so we don't get a warning about it being unused when re-running
# CMake on the same build folder.
message(STATUS "vcpkg integration will expected to be configured by toolchain: ${CMAKE_TOOLCHAIN_FILE}")

project(C99_to_C89)

set(C99_SOURCE_MAIN_ROOT "${CMAKE_SOURCE_DIR}/src/main")
set(C99_SOURCE_TEST_ROOT "${CMAKE_SOURCE_DIR}/src/test")

# Build steps for `c99conv`.
add_executable(c99conv ${C99_SOURCE_MAIN_ROOT}/convert.c)

# Clang is provided by the LLVM project, which gets installed by vcpkg.
find_package(Clang CONFIG REQUIRED)

target_include_directories(c99conv PRIVATE ${CLANG_INCLUDE_DIRS})

# Link against Clang library
target_link_libraries(c99conv PRIVATE libclang)

# Build steps for `c99wrap`.
add_executable(c99wrap ${C99_SOURCE_MAIN_ROOT}/compilewrap.c)

# Add tests as custom targets
add_custom_target(test1
    COMMAND ${CMAKE_C_COMPILER} -E ${C99_SOURCE_TEST_ROOT}/unit.c > ${PROJECT_BINARY_DIR}/unit.prev.c
    COMMAND $<TARGET_FILE:c99conv> ${PROJECT_BINARY_DIR}/unit.prev.c ${PROJECT_BINARY_DIR}/unit.post.c
    COMMAND diff -u ${PROJECT_BINARY_DIR}/unit.prev.c ${PROJECT_BINARY_DIR}/unit.post.c || :
    DEPENDS c99conv
    COMMENT "Running test1"
)

add_custom_target(test2
    COMMAND ${CMAKE_C_COMPILER} -E ${C99_SOURCE_TEST_ROOT}/unit2.c > ${PROJECT_BINARY_DIR}/unit2.prev.c
    COMMAND $<TARGET_FILE:c99conv> ${PROJECT_BINARY_DIR}/unit2.prev.c ${PROJECT_BINARY_DIR}/unit2.post.c
    COMMAND diff -u ${PROJECT_BINARY_DIR}/unit2.prev.c ${PROJECT_BINARY_DIR}/unit2.post.c || :
    DEPENDS c99conv
    COMMENT "Running test2"
)

add_custom_target(test3
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -E ${C99_SOURCE_MAIN_ROOT}/convert.c > ${PROJECT_BINARY_DIR}/convert.prev.c
    COMMAND $<TARGET_FILE:c99conv> ${PROJECT_BINARY_DIR}/convert.prev.c ${PROJECT_BINARY_DIR}/convert.post.c
    COMMAND diff -u ${PROJECT_BINARY_DIR}/convert.prev.c ${PROJECT_BINARY_DIR}/convert.post.c || :
    DEPENDS c99conv
    COMMENT "Running test3"
)
