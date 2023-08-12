cmake_minimum_required(VERSION 3.16.3)
project(mole)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
# add_definitions(${LLVM_DEFINITIONS_LIST})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core)

include_directories(include)
include_directories(${LLVM_INCLUDE_DIRS})
message(STATUS "${LLVM_INCLUDE_DIRS}")

add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_20)

target_compile_options(compiler_flags
    INTERFACE
    "-Wall;-Wextra;-Wno-unused-parameter;-Wno-shadow"
    "-D_GNU_SOURCE;-D__STDC_CONSTANT_MACROS;-D__STDC_FORMAT_MACROS;-D__STDC_LIMIT_MACROS"
    "-Werror"
    "-Wpedantic"
    "-Wformat=2"

    # "-fno-exceptions"
)

target_link_options(compiler_flags
    INTERFACE
    "-fsanitize=undefined"
    "-fsanitize=address"
)
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")

set(HEADERS
    "ast.hpp"
    "exceptions.hpp"
    "lexer.hpp"
    "locale.hpp"
    "operators.hpp"
    "parser.hpp"
    "print_visitor.hpp"
    "reader.hpp"
    "semantic_checker.hpp"
    "string_builder.hpp"
    "token.hpp"
    "types.hpp"
    "visitor.hpp"
)
set(SOURCES
    "lexer.cpp"
    "parser.cpp"
    "print_visitor.cpp"
    "reader.cpp"
    "semantic_checker.cpp"
    "token.cpp"
)
set(TEST_SOURCES
    "reader_tests.cpp"
    "lexer_tests.cpp"
    "parser_tests.cpp"
    "semantic_tests.cpp"
)

list(TRANSFORM HEADERS PREPEND "${INCLUDE}/")
list(TRANSFORM SOURCES PREPEND "${SRC}/")

# list(TRANSFORM TEST_SOURCES PREPEND "${TEST_DIR}/")
add_library(mole ${HEADERS} ${SOURCES})
target_include_directories(mole PUBLIC "${INCLUDE}" "${LLVM_INCLUDE_DIRS}")
target_link_libraries(mole PUBLIC LLVM compiler_flags)

option(COVERAGE "Compile tests with coverage flags." FALSE)
option(TESTS "Compile the tests executables." TRUE)

if(NOT COVERAGE)
    add_executable(out main.cpp)
    target_link_libraries(out PRIVATE mole)
endif()

if(TESTS)
    message(STATUS "Compiling tests.")

    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG origin/devel)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR/extras})
    include(CTest)
    include(Catch)

    if(COVERAGE)
        message(STATUS "Compiling with coverage info.")

        include(CodeCoverage)
        append_coverage_compiler_flags_to_target(mole)
    endif()

    foreach(TEST_SRC IN LISTS TEST_SOURCES)
        get_filename_component(TEST_EXE "${TEST_SRC}" NAME_WE)
        add_executable("${TEST_EXE}" "${TEST_DIR}/${TEST_SRC}")
        target_link_libraries("${TEST_EXE}" PRIVATE mole Catch2::Catch2WithMain)
        catch_discover_tests("${TEST_EXE}")
    endforeach()
endif()