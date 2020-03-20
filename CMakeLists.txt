cmake_minimum_required(VERSION 3.6)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(ivcmp)

set(PROJECT_EXAMPLE_NAME ${PROJECT_NAME}example)
set(PROJECT_LIB_NAME ${PROJECT_NAME})

# Project, library
add_library(${PROJECT_LIB_NAME} SHARED ivcmp.c)
add_library(${PROJECT_LIB_NAME}_static OBJECT ivcmp.c)  # and static library
include(GNUInstallDirs)
set_target_properties(${PROJECT_LIB_NAME} PROPERTIES
    PUBLIC_HEADER ivcmp.h)
if(NOT WIN32)
    install(TARGETS ${PROJECT_LIB_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

# Project itself, example
add_executable(${PROJECT_EXAMPLE_NAME} main.c)
target_link_libraries(${PROJECT_EXAMPLE_NAME} ${PROJECT_LIB_NAME})
add_dependencies(${PROJECT_EXAMPLE_NAME} ${PROJECT_LIB_NAME})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_EXAMPLE_NAME})

# Link Math
if(NOT WIN32)
    target_link_libraries(${PROJECT_LIB_NAME} m)
    target_link_libraries(${PROJECT_EXAMPLE_NAME} m)
endif()