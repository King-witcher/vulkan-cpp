# ----------------------------------------------------------------------------
#  Módulo: engine  —  janela, input, device, swapchain, pipeline, etc.
#  Biblioteca estática que junta o seu código de motor sobre core/Vulkan/SDL3.
# ----------------------------------------------------------------------------

# Coleta todos os .cc de src/. CONFIGURE_DEPENDS faz o CMake reconfigurar
# automaticamente quando você adiciona/remove um arquivo de origem.
file(GLOB ENGINE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_library(engine STATIC ${ENGINE_SRC})
add_library(VkWizard::engine ALIAS engine)

# Headers públicos do engine (engine.h, window.h, ...). PUBLIC = visíveis para o
# próprio engine E para quem o consome (o game). Preserva os #include "engine.h"
# etc. resolvendo, sem precisar mexer no código.
target_include_directories(engine
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# (O padrão C++26 vem de CMAKE_CXX_STANDARD, definido no CMakeLists raiz.)

# Defines do vulkan.hpp. PUBLIC para que valham em TODA translation unit que
# inclui <vulkan/vulkan_raii.hpp> (engine e game) — o header é header-only, então
# os defines têm de estar ativos em todo consumidor, senão há ODR violation.
# Como -D na linha de compilação, ficam garantidos antes de qualquer #include.
target_compile_definitions(engine PUBLIC
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)

# Dependências PUBLIC porque aparecem nos headers públicos do engine
# (vários headers incluem vulkan_raii.hpp; Window.h usa SDL; usam tipos de core).
# Sendo PUBLIC, o game herda Vulkan/SDL3/core de graça só por linkar o engine.
target_link_libraries(engine
    PUBLIC VkWizard::core SDL3::SDL3 glm::glm
    PRIVATE Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
