set(LIB_HEADERS
    "semantic_checker.hpp"
)
set(LIB_SOURCES
    "semantic_checker.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
add_library(mole_semantic_checker
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_semantic_checker PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_semantic_checker PUBLIC mole_ast mole_parser mole_utils)
target_link_libraries(mole_semantic_checker PUBLIC compiler_flags)