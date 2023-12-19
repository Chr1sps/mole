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

include(FetchContent)

# FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(mole_print_visitor PUBLIC nlohmann_json::nlohmann_json)