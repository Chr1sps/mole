set(LIB_HEADERS
    "logger.hpp"
)
set(LIB_SOURCES
    "logger.cpp"
)
list(TRANSFORM LIB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/include/")
list(TRANSFORM LIB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
add_library(mole_logger
    "${LIB_HEADERS}"
    "${LIB_SOURCES}"
)

target_include_directories(mole_logger PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mole_logger PUBLIC mole_utils)
target_link_libraries(mole_logger PUBLIC compiler_flags)