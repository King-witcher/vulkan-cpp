# -----------------------------------------------------------------------------
#  Render Hardware Interface
#  Engine consumes, backends implement.
# -----------------------------------------------------------------------------
add_library(rhi INTERFACE)
add_library(VkWizard::rhi ALIAS rhi)

target_include_directories(rhi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(rhi INTERFACE VkWizard::core)
