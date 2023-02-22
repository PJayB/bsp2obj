cmake_minimum_required(VERSION 3.1)

project(bsp2obj)

find_package(PhysFS REQUIRED)

add_executable(bsp2obj
    BSP.cpp
    OBJ.cpp
    bsp2obj.cpp
    )

target_include_directories(bsp2obj
    PRIVATE ${PHYSFS_INCLUDE_DIR}
    )

target_link_libraries(bsp2obj
    PUBLIC ${PHYSFS_LIBRARY}
    )
