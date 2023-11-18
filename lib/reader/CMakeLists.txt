set(LIB_HEADERS
    "reader.hpp"
)
set(LIB_SOURCES
    "reader.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
add_library(mole_reader
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_reader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_reader PUBLIC mole_utils)
target_link_libraries(mole_reader PUBLIC compiler_flags)