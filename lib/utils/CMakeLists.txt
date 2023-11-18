set(LIB_HEADERS
    "exceptions.hpp"
    "locale.hpp"
    "position.hpp"
    "string_builder.hpp"
)

set(LIB_SOURCES
    "position.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "src/")
add_library(mole_utils
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_utils PUBLIC compiler_flags)
