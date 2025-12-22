cmake_minimum_required(VERSION 4.2.1)
project(VkWizardVS CXX)

set(CMAKE_CXX_STANDARD 26)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/headers
    ${CMAKE_SOURCE_DIR}/external/vulkan/include
    ${CMAKE_SOURCE_DIR}/external/SDL3/include
)

link_directories(
    ${CMAKE_SOURCE_DIR}/external/vulkan/lib
    ${CMAKE_SOURCE_DIR}/external/SDL3/lib
)

file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS "src/sources/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    vulkan-1
    SDL3
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} gdi32)
endif()
