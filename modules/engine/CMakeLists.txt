# ----------------------------------------------------------------------------
#  Módulo: engine  —  janela, input, device, swapchain, pipeline, etc.
#  Biblioteca estática que junta o seu código de motor sobre core/Vulkan/SDL3.
# ----------------------------------------------------------------------------

# Coleta todos os .cpp de src/. CONFIGURE_DEPENDS faz o CMake reconfigurar
# automaticamente quando você adiciona/remove um arquivo de origem.
file(GLOB ENGINE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(engine STATIC ${ENGINE_SRC})
add_library(VkWizard::engine ALIAS engine)

# Headers públicos do engine (Engine.h, Window.h, ...). PUBLIC = visíveis para o
# próprio engine E para quem o consome (o game). Preserva os #include "Engine.h"
# etc. resolvendo, sem precisar mexer no código.
target_include_directories(engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# (O padrão C++26 vem de CMAKE_CXX_STANDARD, definido no CMakeLists raiz.)

# Dependências PUBLIC porque aparecem nos headers públicos do engine
# (Vulkan.h inclui vulkan; Window.h usa SDL; vários headers usam tipos de core).
# Sendo PUBLIC, o game herda Vulkan/SDL3/core de graça só por linkar o engine.
target_link_libraries(engine PUBLIC
    VkWizard::core
    Vulkan::Vulkan
    SDL3::SDL3
    glm::glm)
