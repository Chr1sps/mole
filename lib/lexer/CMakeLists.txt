set(LIB_HEADERS
    "lexer.hpp"
    "token.hpp"
)
set(LIB_SOURCES
    "lexer.cpp"
    "token.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
add_library(mole_lexer
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_lexer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_lexer PUBLIC mole_reader mole_utils mole_logger)
target_link_libraries(mole_lexer PUBLIC compiler_flags)