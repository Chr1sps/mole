set(LIB_HEADERS
    "parser.hpp"
)
set(LIB_SOURCES
    "parser.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
add_library(mole_parser
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_parser PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_parser PUBLIC mole_ast mole_lexer mole_utils)
target_link_libraries(mole_parser PUBLIC compiler_flags)