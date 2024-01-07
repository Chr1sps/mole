set(LIB_HEADERS
    "ir_generator.hpp"
)

set(LIB_SOURCES
    "ir_generator.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

add_library(mole_ir_generator
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_ir_generator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_ir_generator PUBLIC mole_ast)
target_link_libraries(mole_ir_generator PUBLIC compiler_flags)