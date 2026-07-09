# ----------------------------------------------------------------------------
#  Módulo: rhi_vk  —  implementação Vulkan da RHI.
#  Compila todo o backend (device, swapchain, pipeline, allocator, driver).
#  Só expõe rhi_vk.h (a fábrica); nada de Vulkan vaza para os consumidores.
# ----------------------------------------------------------------------------
file(GLOB RHI_VK_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

add_library(rhi_vk STATIC ${RHI_VK_SRC})
add_library(VkWizard::rhi_vk ALIAS rhi_vk)

# rhi_vk.h (raiz) é público; os headers de implementação (src/) são privados.
target_include_directories(rhi_vk
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src)

# Defines do vulkan.hpp. PRIVATE: só as TUs internas do backend incluem
# vulkan.hpp; o header público não, então os defines não precisam vazar.
target_compile_definitions(rhi_vk PRIVATE
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)

# rhi é PUBLIC porque rhi_vk.h inclui rhi.h (a interface faz parte do contrato).
# Vulkan/VMA/SDL3 são PRIVATE: detalhes de implementação do backend.
target_link_libraries(rhi_vk
    PUBLIC  VkWizard::core VkWizard::rhi
    PRIVATE Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator SDL3::SDL3)
