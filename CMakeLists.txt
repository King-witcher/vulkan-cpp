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

# Arquivos fonte
file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS "src/sources/*.cpp")

# Executável
add_executable(${PROJECT_NAME} ${SOURCES})

# Linkagem das bibliotecas
target_link_libraries(${PROJECT_NAME}
    vulkan-1
    SDL3
)

# Configurações específicas do Windows
if(WIN32)
    target_link_libraries(${PROJECT_NAME} gdi32)
endif()
