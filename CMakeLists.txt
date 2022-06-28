cmake_minimum_required(VERSION 3.0.2)

project(ivcmp)

set(PROJECT_EXAMPLE_NAME ${PROJECT_NAME}example)
set(PROJECT_LIB_NAME ${PROJECT_NAME})

# Project, library
add_library(${PROJECT_LIB_NAME} SHARED src/ivcmp.c)
add_library(${PROJECT_LIB_NAME}_static OBJECT src/ivcmp.c)  # and static library
include(GNUInstallDirs)
set_target_properties(${PROJECT_LIB_NAME} PROPERTIES
    PUBLIC_HEADER src/ivcmp.h)
if(NOT WIN32)
    install(TARGETS ${PROJECT_LIB_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

# Project itself, example
add_executable(${PROJECT_EXAMPLE_NAME} src/main.c)
target_link_libraries(${PROJECT_EXAMPLE_NAME} ${PROJECT_LIB_NAME})
add_dependencies(${PROJECT_EXAMPLE_NAME} ${PROJECT_LIB_NAME})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_EXAMPLE_NAME})

# Threat all warnings as errors
if(MSVC)
    target_compile_options(${PROJECT_LIB_NAME} PRIVATE /W4 /WX)
    target_compile_options(${PROJECT_EXAMPLE_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_LIB_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(${PROJECT_EXAMPLE_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Link Math
if(NOT WIN32)
    target_link_libraries(${PROJECT_LIB_NAME} m)
    target_link_libraries(${PROJECT_EXAMPLE_NAME} m)
endif()

# Treat Warnings as Errors
if(MSVC)
  target_compile_options(${PROJECT_LIB_NAME} PRIVATE /W3 /WX)
  target_compile_options(${PROJECT_EXAMPLE_NAME} PRIVATE /W3 /WX)
else()
  target_compile_options(${PROJECT_LIB_NAME} PRIVATE -Wall -Wextra -Werror)
  target_compile_options(${PROJECT_EXAMPLE_NAME} PRIVATE -Wall -Wextra -Werror)
endif()