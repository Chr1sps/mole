set(LIB_HEADERS
    "compiled_program.hpp"
)

set(LIB_SOURCES
    "compiled_program.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

add_library(mole_compiled_program
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_compiled_program PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_compiled_program PUBLIC mole_ast)
target_link_libraries(mole_compiled_program PUBLIC compiler_flags)