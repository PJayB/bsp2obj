cmake_minimum_required(VERSION 3.1)

project(bsp2obj)

find_package(PhysFS REQUIRED)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/missing.tga.h
    COMMAND xxd -i missing.tga > ${CMAKE_BINARY_DIR}/missing.tga.h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
    )

add_custom_target(missing_tga
    DEPENDS ${CMAKE_BINARY_DIR}/missing.tga.h
    )

add_executable(bsp2obj
    BSP.cpp
    OBJ.cpp
    bsp2obj.cpp
    )

add_dependencies(bsp2obj missing_tga)

target_include_directories(bsp2obj
    PRIVATE ${PHYSFS_INCLUDE_DIR}
    PRIVATE ${CMAKE_BINARY_DIR}
    )

target_link_libraries(bsp2obj
    PUBLIC ${PHYSFS_LIBRARY}
    )
