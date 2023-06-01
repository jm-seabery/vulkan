project(app-test)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCES_APP
    ${PROJECT_SOURCE_DIR}/main.cpp
)

# all sources
list(APPEND COMBINED_SOURCES ${SOURCES_EXT} ${SOURCES_APP})
list(APPEND PROJECT_TREE_SOURCES ${SOURCES_APP})

# Add the executable
add_executable(${PROJECT_NAME} ${COMBINED_SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_property(TARGET ${PROJECT_NAME} PROPERTY RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})
set_property(TARGET ${PROJECT_NAME} PROPERTY RELWITHDEBINFO_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_TREE_SOURCES})

# Specify linked libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} glfw)
    target_link_libraries(${PROJECT_NAME} SDL2-static)
    target_link_libraries(${PROJECT_NAME} glslang)
    target_link_libraries(${PROJECT_NAME} glslang-default-resource-limits)
    target_link_libraries(${PROJECT_NAME} SPIRV)
    target_link_libraries(${PROJECT_NAME} vulkan)

    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

elseif(APPLE)
elseif(WIN32)
    if(MSVC)
    else()
    endif()
endif()

include_directories(${PROJECT_SOURCE_DIR})

# This sets the output folder for an install
install(TARGETS ${PROJECT_NAME} DESTINATION bin/)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/" # source directory
    DESTINATION "bin" # target directory
)
