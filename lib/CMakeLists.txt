set(SUBDIRS ast ir_generator lexer logger parser json_serializer reader semantic_checker utils)

foreach(SUBDIR IN LISTS SUBDIRS)
    add_subdirectory("${SUBDIR}")
endforeach()

add_library(mole INTERFACE)
target_link_libraries(mole INTERFACE
    mole_ast
    mole_ir_generator
    mole_json_serializer
    mole_lexer
    mole_logger
    mole_parser
    mole_reader
    mole_semantic_checker
    mole_utils
)