set(LIB_HEADERS
    "ast.hpp"
    "visitor.hpp"
)
set(LIB_SOURCES
    ""
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
add_library(mole_ast
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_ast PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_ast PUBLIC mole_utils)
target_link_libraries(mole_ast PUBLIC compiler_flags)