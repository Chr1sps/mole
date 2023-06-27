cmake_minimum_required(VERSION 3.16.3)
project(mole)

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

target_link_options(compiler_flags INTERFACE "-fsanitize=undefined;-fsanitize=address")
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(HEADERS

    "ast.hpp"
    "debug_visitor.hpp"
    "lexer.hpp"
    "locale.hpp"
    "operators.hpp"
    "parser.hpp"
    "print_visitor.hpp"
    "reader.hpp"
    "token.hpp"
    "types.hpp"
    "visitor.hpp"
)
set(SOURCES

    "debug_visitor.cpp"
    "lexer.cpp"
    "parser.cpp"
    "print_visitor.cpp"
    "token.cpp"
)

list(TRANSFORM HEADERS PREPEND "${INCLUDE}/")
list(TRANSFORM SOURCES PREPEND "${SRC}/")

add_library(mole ${HEADERS} ${SOURCES})
target_include_directories(mole PUBLIC "${INCLUDE}")
target_include_directories(mole PUBLIC "${LLVM_INCLUDE_DIRS}")
target_link_libraries(mole PUBLIC LLVM)
target_link_libraries(mole PUBLIC compiler_flags)

add_executable(out main.cpp)
target_link_libraries(out PRIVATE mole)

option(COVERAGE "Compile tests with coverage flags." FALSE)
option(TESTS "Compile the tests executables." TRUE)

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

    add_executable(lexer_tests tests/lexer_tests.cpp)
    target_link_libraries(lexer_tests PRIVATE mole Catch2::Catch2WithMain)

    add_executable(parser_tests tests/parser_tests.cpp)
    target_link_libraries(parser_tests PRIVATE mole Catch2::Catch2WithMain)

    catch_discover_tests(lexer_tests)
    catch_discover_tests(parser_tests)

    if(COVERAGE)
        message(STATUS "Compiling with coverage info.")
        add_library(coverage_flags INTERFACE)
        target_compile_options(coverage_flags INTERFACE "--coverage")
        target_link_options(coverage_flags INTERFACE "--coverage")

        target_link_libraries(lexer_tests PRIVATE coverage_flags)
        target_link_libraries(mole PRIVATE coverage_flags)
    endif()
endif()