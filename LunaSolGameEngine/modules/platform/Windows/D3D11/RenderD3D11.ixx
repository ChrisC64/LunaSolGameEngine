module;
#include <cstdint>
#include <array>
#include <span>
#include <filesystem>
#include <wrl/client.h>
#include <d3d11_4.h>
#include "engine/EngineDefines.h"
export module D3D11.RenderD3D11;

import Engine.LSWindow;
import Engine.LSDevice;
import Engine.EngineCodes;
import D3D11.PipelineFactory;
import D3D11.Device;
import D3D11.Utils;
import D3D11.PipelineFactory;
import DirectXCommon;

namespace LS
{
    export struct TimeStep
    {
        uint64_t ElapsedMs;
        uint64_t DeltaMs;
    };

    export class IRenderable
    {
    public:
        IRenderable() noexcept;
        virtual ~IRenderable() noexcept;

        virtual void Draw() noexcept = 0;
        virtual void Update(const LS::DX::DXCamera& camera, const TimeStep& timeStep) noexcept = 0;
    };

}

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    enum class COMMAND_MODE
    {
        IMMEDIATE,
        DEFERRED
    };

    enum class SHADER_RESOURCE_TYPE
    {
        CBUFFER,
        SAMPLER,
        TEXTURE,
        UAV
    };

    template <class T>
    struct ShaderResource
    {
        uint32_t Slot;
        T Resource;
        std::string Name;

        constexpr bool operator==(uint32_t slot) const
        {
            return Slot == slot;
        }
        
        constexpr bool operator==(const char* c) const
        {
            return Name == c;
        }
    };
    //TODO: 3-10-24 Create and implement the definitions for the functions below
    template <class T>
    using ShaderArray = std::vector<ShaderResource<T>>;

    template <class T>
    class ShaderD3D11
    {
    public:
        ShaderD3D11() = default;
        ~ShaderD3D11() = default;

        auto GetConstantBuffer(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>;
        auto GetConstantBuffer(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>;
        auto GetTexture(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>;
        auto GetTexture(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>;
        auto GetSampler(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>;
        auto GetSampler(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>;
        auto GetUav(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>;
        auto GetUav(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>;

    private:
        T m_shader;
        ShaderArray<WRL::ComPtr<ID3D11Buffer>> m_constantBuffers;
        ShaderArray<WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
        ShaderArray<WRL::ComPtr<ID3D11SamplerState>> m_samplers;
        ShaderArray<WRL::ComPtr<ID3D11UnorderedAccessView>> m_uavs;
    };

    using VertexShaderDx11 = ShaderD3D11<WRL::ComPtr<ID3D11VertexShader>>;
    using PixelShaderDx11 = ShaderD3D11<WRL::ComPtr<ID3D11PixelShader>>;
    using GeometryShaderDx11 = ShaderD3D11<WRL::ComPtr<ID3D11GeometryShader>>;
    using ComputeShaderDx11 = ShaderD3D11<WRL::ComPtr<ID3D11ComputeShader>>;
    using DomainShaderDx11 = ShaderD3D11<WRL::ComPtr<ID3D11DomainShader>>;
    using HullShaderDx11 = ShaderD3D11<WRL::ComPtr<ID3D11HullShader>>;

    class RenderCommandD3D11
    {
    public:
        RenderCommandD3D11(ID3D11Device* pDevice, COMMAND_MODE mode);
        ~RenderCommandD3D11();

        // Bind Shaders to Pipeline //
        void BindVS(const VertexShaderDx11& shader) noexcept;
        void BindPS(const PixelShaderDx11& shader) noexcept;
        void BindGS(const GeometryShaderDx11& shader) noexcept;
        void BindCS(const ComputeShaderDx11& shader) noexcept;
        void BindHS(const HullShaderDx11& shader) noexcept;
        void BindDS(const DomainShaderDx11& shader) noexcept;

        // Bind Commands for Resources for Shaders //
        void UpdateTexture(ID3D11Resource* resource, const void* data) noexcept;
        void UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data) noexcept;

        // Input Assembly Functions //
        void SetInputLayout(ID3D11InputLayout* il) noexcept;
        void SetVertexBuffer(ID3D11Buffer* vb) noexcept;
        void SetIndexBuffer(ID3D11Buffer* ib) noexcept;
        void SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept;

        // Output Merger Functions //
        void SetRenderTargets(std::span<ID3D11RenderTargetView*> rtvs) noexcept;
        void SetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef = 0) noexcept;
        void SetBlendState(ID3D11BlendState* bs, uint32_t sampleMask = 0xffffffff, std::array<float, 4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f }) noexcept;

        // Set Input For this Draw State //
        void SetRasterizerState(ID3D11RasterizerState* rss) noexcept;
        void SetViewPort(std::span<D3D11_VIEWPORT> viewports) noexcept;

        // Draw Commands //
        void Clear(const float* rgbaColor) noexcept;
        void ClearDepth(ID3D11DepthStencilView* dsv) noexcept;
        void DrawIndexed(uint32_t indexCount, uint32_t indexOffset = 0,
            uint32_t vertexOffset = 0) noexcept;
        void DrawIndxInstances(uint32_t indexCount, uint32_t instances, uint32_t indexOffset = 0,
            uint32_t baseOffset = 0, uint32_t instanceOffset = 0) noexcept;
        void DrawVerts(uint32_t vertexCount, uint32_t vertexOffset = 0) noexcept;
        void DrawVertInstances(uint32_t vertexCount, uint32_t instances,
            uint32_t vertexOffset = 0, uint32_t instanceOffset = 0) noexcept;
        // State Operations //
        void ClearState() noexcept; // @brief Resets to default state
        void FlushCommands() noexcept;// @brief Expunge all commands recorded up to this point

        auto GetMode() -> COMMAND_MODE
        {
            return m_mode;
        }

        auto GetContext() -> ID3D11DeviceContext*
        {
            return m_context.Get();
        }

    private:
        WRL::ComPtr<ID3D11DeviceContext> m_context;
        COMMAND_MODE m_mode;
    };

    class RenderD3D11
    {
    public:
        RenderD3D11(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow);
        ~RenderD3D11();

        auto Initialize() noexcept -> LS::System::ErrorCode;
        void Clear(std::array<float, 4> clearColor) noexcept;
        void SetPipeline(const PipelineStateDX11* state) noexcept;
        void LoadVertexBuffer(uint32_t startSlot, const PipelineStateDX11* state) noexcept;
        void LoadIndexBuffer(uint32_t offset, const PipelineStateDX11* state) noexcept;
        void Update(const LS::DX::DXCamera& camera) noexcept;
        void RenderObjects(std::span<LS::IRenderable*> objs) noexcept;
        void Draw() noexcept;
        void Shutdown() noexcept;
        void Resize(uint32_t width, uint32_t height) noexcept;
        void AttachToWindow(LS::LSWindowBase* window) noexcept;

        auto GetDevice() noexcept -> ID3D11Device*;
        auto GetDeviceCom() noexcept -> WRL::ComPtr<ID3D11Device>;
        auto GetSwapChainCom() noexcept -> WRL::ComPtr<IDXGISwapChain1>;
        auto GetDeviceContextCom() noexcept -> WRL::ComPtr<ID3D11DeviceContext>;

        auto CreateVertexShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11VertexShader>;
        auto CreatePixelShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11PixelShader>;
        auto CreateGeometryShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11GeometryShader>;
        auto CreateDomainShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11DomainShader>;
        auto CreateHullShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11HullShader>;
        auto CreateComputeShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11ComputeShader>;

        /**
         * @brief Creates an input layout from the given compiled bytecode. This will not work if it was not compiled first.
         *
         * @param compiledByteCode The compiled byte code of the shader
         * @return Nullable<WRL::ComPtr<ID3D11InputLayout>> A nullable object of WRL::ComPtr<ID3D11InputLayout>>
         */
        auto BuildInputLayout(std::span<std::byte> compiledByteCode) -> Nullable<WRL::ComPtr<ID3D11InputLayout>>;

        auto ExecuteRenderCommand(const RenderCommandD3D11& command) noexcept;
    protected:
        LS::LSWindowBase* m_window;
        DeviceD3D11             m_device;
        LS::LSDeviceSettings    m_settings;

        WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;
        WRL::ComPtr<ID3D11DeviceContext> m_context;
    };

}

module : private;

using namespace LS::Win32;

#define FIND_OR_NULL(c, o, exp) \
for (const auto& o : c) \
{ if (exp) return o; } \
return std::nullopt

template <class T, class C>
constexpr auto FindOrNull(const C& container, const auto& query) -> Nullable<T>
{
    for (const auto& obj : container)
    {
        if (obj == query)
        {
            return obj.Resource;
        }
    }
    return std::nullopt;
}


template<class T>
auto LS::Win32::ShaderD3D11<T>::GetConstantBuffer(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>
{
    return FindOrNull<WRL::ComPtr<ID3D11Buffer>>(m_constantBuffers, id.data());
}

template<class T>
auto LS::Win32::ShaderD3D11<T>::GetConstantBuffer(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>
{
    return FindOrNull<WRL::ComPtr<ID3D11Buffer>>(m_constantBuffers, slot);
}

template<class T>
auto LS::Win32::ShaderD3D11<T>::GetTexture(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>
{
    return FindOrNull<WRL::ComPtr<ID3D11ShaderResourceView>>(m_textures, id.data());
}

template<class T>
auto LS::Win32::ShaderD3D11<T>::GetTexture(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>
{
    return FindOrNull<WRL::ComPtr<ID3D11ShaderResourceView>>(m_textures, slot);
}

template<class T>
auto LS::Win32::ShaderD3D11<T>::GetSampler(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>
{
    return FindOrNull<WRL::ComPtr<ID3D11SamplerState>>(m_samplers, id.data());
}

template<class T>
auto LS::Win32::ShaderD3D11<T>::GetSampler(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>
{
    return FindOrNull<WRL::ComPtr<ID3D11SamplerState>>(m_samplers, slot);
}

template<class T>
auto LS::Win32::ShaderD3D11<T>::GetUav(std::string_view id) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>
{
    return FindOrNull<WRL::ComPtr<ID3D11UnorderedAccessView>>(m_uavs, id.data());
}

template<class T>
auto LS::Win32::ShaderD3D11<T>::GetUav(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>
{
    return FindOrNull<WRL::ComPtr<ID3D11UnorderedAccessView>>(m_uavs, slot);
}