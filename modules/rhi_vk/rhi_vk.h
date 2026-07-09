#pragma once

// ---------------------------------------------------------------------------
//  rhi_vk — backend Vulkan da RHI.
//
//  Único header público do backend. Não expõe NADA de Vulkan: só a fábrica que
//  devolve um gd::rhi::Driver pronto. Depende da SDL apenas para criar a
//  surface a partir da janela (a windowing layer do projeto é SDL3).
// ---------------------------------------------------------------------------

#include <memory>

#include "rhi.h"

struct SDL_Window;

namespace gd::rhi::vulkan
{
    // Cria um driver Vulkan que renderiza na janela SDL dada. O backend cria por
    // conta própria instance, device, swapchain e allocator.
    std::unique_ptr<Driver> CreateDriver(SDL_Window *window);
} // namespace gd::rhi::vulkan
