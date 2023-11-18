set(LIB_HEADERS
    "print_visitor.hpp"
)
set(LIB_SOURCES
    "print_visitor.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
add_library(mole_print_visitor
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_print_visitor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_print_visitor PUBLIC mole_ast)
target_link_libraries(mole_print_visitor PUBLIC compiler_flags)