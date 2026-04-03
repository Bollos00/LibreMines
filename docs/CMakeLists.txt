find_package(Doxygen REQUIRED dot)

doxygen_add_docs(
    documentation
    "${CMAKE_CURRENT_SOURCE_DIR}/../src" "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    COMMENT "Generating LibreMines documentation with Doxygen"
    ALL
)

# Documentation validation target - TODO Check which files/classes/functions/members are missing documentation and generate coverage report