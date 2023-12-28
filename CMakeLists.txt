cmake_minimum_required(VERSION 3.16.3)
project(mole)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core)

include_directories(${LLVM_INCLUDE_DIRS})
message(STATUS "${LLVM_INCLUDE_DIRS}")

add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_23)

target_compile_options(compiler_flags
    INTERFACE
    "-Wall;-Wextra;-Wno-unused-parameter;-Wno-shadow"
    "-D_GNU_SOURCE;-D__STDC_CONSTANT_MACROS;-D__STDC_FORMAT_MACROS;-D__STDC_LIMIT_MACROS"
    "-Werror"
    "-Wpedantic"
    "-Wformat=2"
    "-ftemplate-backtrace-limit=0"

    # "-fno-exceptions"
)

target_link_options(compiler_flags
    INTERFACE
    "-fsanitize=undefined"
    "-fsanitize=address"
)

set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
add_subdirectory("${LIB_DIR}")

# add_library(mole ${HEADERS} ${SOURCES})
# target_include_directories(mole PUBLIC "${INCLUDE}" "${LLVM_INCLUDE_DIRS}")
option(COVERAGE "Compile tests with coverage flags." FALSE)
option(TESTS "Compile the tests executables." TRUE)

if(NOT COVERAGE)
    add_executable(molec main.cpp)

    target_link_libraries(molec PRIVATE mole)
    target_link_libraries(molec PRIVATE LLVM compiler_flags)
endif()

function(new_test)
    cmake_parse_arguments(
        "ARGS"
        ""
        "SOURCE"
        "LIBS"
        "${ARGN}"
    )

    if(NOT ARGS_SOURCE)
        message(FATAL_ERROR "No source provided.")
    endif()

    get_filename_component(TEST_EXE "${ARGS_SOURCE}" NAME_WE)
    add_executable("${TEST_EXE}" "${TEST_DIR}/${ARGS_SOURCE}")
    target_link_libraries("${TEST_EXE}" PRIVATE Catch2::Catch2WithMain)

    foreach(LIB "${ARGS_LIBS}")
        target_link_libraries("${TEST_EXE}" PRIVATE "${LIB}")
    endforeach()

    catch_discover_tests("${TEST_EXE}")
endfunction()

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

    new_test(SOURCE "reader_tests.cpp" LIBS mole_reader)
    new_test(SOURCE "lexer_tests.cpp" LIBS mole_lexer)
    new_test(SOURCE "parser_tests.cpp" LIBS mole_parser)
    new_test(SOURCE "semantic_tests.cpp" LIBS mole_semantic_checker)
endif()