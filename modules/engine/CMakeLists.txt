# ----------------------------------------------------------------------------
#  Módulo: engine  —  janela, input, device, swapchain, pipeline, etc.
#  Biblioteca estática que junta o seu código de motor sobre core/Vulkan/SDL3.
# ----------------------------------------------------------------------------

# Coleta todos os .cc de src/. CONFIGURE_DEPENDS faz o CMake reconfigurar
# automaticamente quando você adiciona/remove um arquivo de origem.
file(GLOB ENGINE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

add_library(engine STATIC ${ENGINE_SRC})
add_library(VkWizard::engine ALIAS engine)

# Headers públicos do engine (engine.h, window.h, ...). PUBLIC = visíveis para o
# próprio engine E para quem o consome (o game). Preserva os #include "engine.h"
# etc. resolvendo, sem precisar mexer no código.
target_include_directories(engine
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src)

# (O padrão C++26 vem de CMAKE_CXX_STANDARD, definido no CMakeLists raiz.)

# O engine não fala mais Vulkan diretamente: ele programa contra a RHI (rhi) e
# escolhe o backend (rhi_vk) na fábrica. Tudo PRIVATE porque nada disso aparece
# no header público do engine (include/engine/engine.h só usa <memory>); o game
# depende apenas do engine. SDL3/glm são usados nos .cc/headers privados.
target_link_libraries(engine
    PRIVATE VkWizard::core VkWizard::rhi VkWizard::rhi_vk SDL3::SDL3 glm::glm)
