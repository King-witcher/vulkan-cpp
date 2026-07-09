#pragma once

// ---------------------------------------------------------------------------
//  gd::rhi — Render Hardware Interface
//
//  Interface abstrata de GPU, sem NENHUMA dependência de backend (nada de
//  Vulkan, SDL_gpu, etc.). O motor programa contra estas classes; cada backend
//  (rhi_vk, futuramente rhi_sdlgpu) fornece uma implementação concreta.
//
//  Modelo:
//    - Recursos (buffers, pipelines) são referenciados por handles opacos.
//      O Driver é o dono da memória; o handle é só um id.
//    - O ciclo de um frame é: BeginFrame() -> grava no RenderPass ->
//    EndFrame().
//      Toda a sincronização (semáforos/fences/barriers no Vulkan; nada no
//      SDL_gpu) fica ESCONDIDA dentro do backend.
// ---------------------------------------------------------------------------

#include <span>

#include "rust_types.h"

namespace gd::rhi
{
    // -- Opaque handles ------------------------------------------------------
    // id == 0 means invalid.
    struct BufferHandle
    {
        u32 id = 0;
        explicit operator bool() const { return id != 0; }
    };

    struct PipelineHandle
    {
        u32 id = 0;
        explicit operator bool() const { return id != 0; }
    };

    enum class VertexFormat
    {
        Float32x2,
        Float32x3,
        Float32x4,
    };

    enum class PrimitiveTopology
    {
        TriangleList,
    };

    enum class CullMode
    {
        None,
        Back,
        Front,
    };

    enum class BufferUsage
    {
        Vertex,
        Index,
    };

    struct VertexAttribute
    {
        u32 location;
        VertexFormat format;
        u32 offset;
    };

    struct VertexLayout
    {
        u32 stride;
        std::span<const VertexAttribute> attributes;
    };

    struct ShaderModuleDesc
    {
        // SPIR-V bytecode
        std::span<const u8> spirv;
        const char *entryPoint;
    };

    struct PipelineDesc
    {
        ShaderModuleDesc vertexShader;
        ShaderModuleDesc fragmentShader;
        VertexLayout vertexLayout;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        CullMode cullMode = CullMode::Back;
    };

    struct BufferDesc
    {
        usize size;
        BufferUsage usage;
    };

    struct ClearColor
    {
        f32 r, g, b, a;
    };

    class RenderPass
    {
    public:
        virtual ~RenderPass() = default;

        virtual void BindPipeline(PipelineHandle) = 0;
        virtual void BindVertexBuffer(BufferHandle) = 0;
        virtual void Draw(u32 vertexCount) = 0;
    };

    class Driver
    {
    public:
        virtual ~Driver() = default;

        virtual BufferHandle CreateBuffer(const BufferDesc &) = 0;
        virtual void WriteBuffer(BufferHandle, std::span<const u8> data) = 0;
        virtual void DestroyBuffer(BufferHandle) = 0;

        virtual PipelineHandle CreatePipeline(const PipelineDesc &) = 0;
        virtual void DestroyPipeline(PipelineHandle) = 0;

        virtual RenderPass &BeginFrame(const ClearColor &) = 0;
        virtual void EndFrame() = 0;

        virtual void WaitIdle() = 0;
    };
} // namespace gd::rhi
