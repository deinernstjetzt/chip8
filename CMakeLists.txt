cmake_minimum_required(VERSION 3.20)
project(c8 C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_search_module(libadwaita REQUIRED IMPORTED_TARGET libadwaita-1)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

add_custom_command(
    OUTPUT gresource.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --target=${CMAKE_CURRENT_BINARY_DIR}/gresource.c
        --generate-source
        resources.xml
    VERBATIM
    MAIN_DEPENDENCY resources.xml
    DEPENDS
        res/appwindow.xml
)

add_custom_target(
    dummy-resource
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gresource.c
)

add_executable(
    c8
    src/main.c 
    src/app.c src/app.h
    src/appwindow.c src/appwindow.h
    src/display.c src/display.h
    src/keystate.c src/keystate.h
    src/cpu.c src/cpu.h
    ${CMAKE_CURRENT_BINARY_DIR}/gresource.c
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/gresource.c
    PROPERTIES GENERATED TRUE
)

add_dependencies(c8 dummy-resource)

target_include_directories(c8 PUBLIC PkgConfig::libadwaita)
target_link_libraries(c8 PUBLIC PkgConfig::libadwaita)