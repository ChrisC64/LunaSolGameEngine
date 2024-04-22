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
import D3D11.RenderFuncD3D11;
import D3D11.MemoryHelper;
import Util.MSUtils;
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
    using ShaderResArray = std::vector<ShaderResource<T>>;

    template <class T>
    class ShaderD3D11
    {
    public:
        ShaderD3D11() = default;
        ~ShaderD3D11() = default;

        auto GetShader() const noexcept
        {
            return m_shader;
        }

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
        ShaderResArray<WRL::ComPtr<ID3D11Buffer>> m_constantBuffers;
        ShaderResArray<WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
        ShaderResArray<WRL::ComPtr<ID3D11SamplerState>> m_samplers;
        ShaderResArray<WRL::ComPtr<ID3D11UnorderedAccessView>> m_uavs;
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
        RenderCommandD3D11() = default;
        RenderCommandD3D11(ID3D11Device* pDevice, COMMAND_MODE mode);
        ~RenderCommandD3D11() = default;

        // Bind Shaders to Pipeline //
        void BindVS(const VertexShaderDx11& shader) noexcept;
        void BindVS(ID3D11VertexShader* vs) noexcept;
        void BindPS(const PixelShaderDx11& shader) noexcept;
        void BindPS(ID3D11PixelShader* ps) noexcept;
        void BindGS(const GeometryShaderDx11& shader) noexcept;
        void BindGS(ID3D11GeometryShader* gs) noexcept;
        void BindCS(const ComputeShaderDx11& shader) noexcept;
        void BindCS(ID3D11ComputeShader* cs) noexcept;
        void BindHS(const HullShaderDx11& shader) noexcept;
        void BindHS(ID3D11HullShader* hs) noexcept;
        void BindDS(const DomainShaderDx11& shader) noexcept;
        void BindDS(ID3D11DomainShader* ds) noexcept;

        // Bind Buffers to Shader Stages Stages //
        void BindVSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindVSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0);
        void BindVSResource(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindVSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot = 0);
        
        void BindPSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindPSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0);
        void BindPSResource(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindPSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot = 0);
        
        void BindGSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindGSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0);
        void BindGSResource(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindGSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot = 0);

        void BindHSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindHSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0);
        void BindHSResource(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindHSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot = 0);

        void BindDSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindDSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0);
        void BindDSResource(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindDSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot = 0);

        void BindCSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindCSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0);
        void BindCSResource(ID3D11Buffer* buffer, uint32_t slot = 0);
        void BindCSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot = 0);

        // Bind Commands for Resources for Shaders //
        void UpdateTexture(ID3D11Resource* resource, const void* data) noexcept;
        void UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data) noexcept;

        // Input Assembly Functions //
        void SetInputLayout(ID3D11InputLayout* il) noexcept;
        void SetVertexBuffers(std::span<ID3D11Buffer*> vbs, std::span<uint32_t> strides, std::span<uint32_t> offsets, uint32_t startSlot = 0) noexcept;
        void SetVertexBuffer(ID3D11Buffer* vb, uint32_t stride, uint32_t startSlot = 0, uint32_t offset = 0) noexcept;
        void SetIndexBuffer(ID3D11Buffer* ib, uint32_t offset = 0, DXGI_FORMAT format= DXGI_FORMAT_R32_UINT) noexcept;
        void SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept;

        // Output Merger Functions //
        void SetRenderTargets(std::span<ID3D11RenderTargetView*> rtvs, ID3D11DepthStencilView* depthStencilView = nullptr) noexcept;
        void SetRenderTarget(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* depthStencilView = nullptr) noexcept;
        void SetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef = 0) noexcept;
        void SetBlendState(ID3D11BlendState* bs, uint32_t sampleMask = 0xffffffff, std::array<float, 4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f }) noexcept;

        // Set Input For this Draw State //
        void SetRasterizerState(ID3D11RasterizerState* rss) noexcept;
        void SetViewports(std::span<D3D11_VIEWPORT> viewports) noexcept;
        void SetViewport(D3D11_VIEWPORT viewports) noexcept;
        void SetViewport(float width, float height) noexcept;

        // Draw Commands //
        void Clear(const float* rgbaColor, ID3D11RenderTargetView* rtv) noexcept;
        void ClearDepth(ID3D11DepthStencilView* dsv, float depth = 1.0f) noexcept;
        void ClearStencil(ID3D11DepthStencilView* dsv, uint8_t stencil = 0) noexcept;
        void ClearDepthStencil(ID3D11DepthStencilView* dsv, float depth = 1.0f, uint8_t stencil = 0) noexcept;
        void DrawIndexed(uint32_t indexCount, uint32_t indexOffset = 0,
            uint32_t vertexOffset = 0) noexcept;
        void DrawIndxInstances(uint32_t indexCount, uint32_t instances, uint32_t indexOffset = 0,
            uint32_t baseOffset = 0, uint32_t instanceOffset = 0) noexcept;
        void DrawVerts(uint32_t vertexCount, uint32_t vertexOffset = 0) noexcept;
        void DrawVertInstances(uint32_t vertexCount, uint32_t instances,
            uint32_t vertexOffset = 0, uint32_t instanceOffset = 0) noexcept;
        // State Operations //
        // @brief Resets to default state
        void ClearState() noexcept; 
        // @brief Expunge all commands recorded up to this point
        void FlushCommands() noexcept;

        auto GetMode() const -> COMMAND_MODE
        {
            return m_mode;
        }

        auto GetContext() const -> ID3D11DeviceContext*
        {
            return m_context.Get();
        }

        auto GetContextComPtr() const -> WRL::ComPtr<ID3D11DeviceContext>
        {
            return m_context;
        }

        auto GetCommandList() const -> Nullable<ID3D11CommandList*>
        {
            if (m_mode == COMMAND_MODE::IMMEDIATE)
                return std::nullopt;
            ID3D11CommandList* pCommList;
            const auto hr = m_context->FinishCommandList(false, &pCommList);
            if (FAILED(hr))
            {
                return std::nullopt;
            }

            return pCommList;
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
        auto CreateImmediateCommand() noexcept -> RenderCommandD3D11;
        auto CreateDeferredCommand() noexcept -> RenderCommandD3D11;
        
        /**
         * @brief Creates an input layout from the given compiled bytecode. This will not work if it was not compiled first.
         *
         * @param compiledByteCode The compiled byte code of the shader
         * @return Nullable<WRL::ComPtr<ID3D11InputLayout>> A nullable object of WRL::ComPtr<ID3D11InputLayout>>
         */
        auto BuildInputLayout(std::span<std::byte> compiledByteCode) -> Nullable<WRL::ComPtr<ID3D11InputLayout>>;

        auto ExecuteRenderCommand(const RenderCommandD3D11& command) noexcept;
    protected:
        LS::LSWindowBase*                       m_window;
        DeviceD3D11                             m_device;
        LS::LSDeviceSettings                    m_settings;

        WRL::ComPtr<ID3D11RenderTargetView>     m_renderTarget;
        WRL::ComPtr<ID3D11DeviceContext>        m_context;
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

RenderCommandD3D11::RenderCommandD3D11(ID3D11Device* pDevice, COMMAND_MODE mode) : m_mode(mode)
{
    if (mode == COMMAND_MODE::IMMEDIATE)
    {
        pDevice->GetImmediateContext(&m_context);
    }
    else
    {
        Utils::ThrowIfFailed(pDevice->CreateDeferredContext(0, &m_context));
    }
}

void RenderCommandD3D11::BindVS(const VertexShaderDx11& shader) noexcept
{
    m_context->VSSetShader(shader.GetShader().Get(), nullptr, 0);
}

void LS::Win32::RenderCommandD3D11::BindVS(ID3D11VertexShader* vs) noexcept
{
    m_context->VSSetShader(vs, nullptr, 0);
}

void RenderCommandD3D11::BindPS(const PixelShaderDx11& shader) noexcept
{
    m_context->PSSetShader(shader.GetShader().Get(), nullptr, 0);
}

void LS::Win32::RenderCommandD3D11::BindPS(ID3D11PixelShader* ps) noexcept
{
    m_context->PSSetShader(ps, nullptr, 0);
}

void RenderCommandD3D11::BindGS(const GeometryShaderDx11& shader) noexcept
{
    m_context->GSSetShader(shader.GetShader().Get(), nullptr, 0);
}

void LS::Win32::RenderCommandD3D11::BindGS(ID3D11GeometryShader* gs) noexcept
{
    m_context->GSSetShader(gs, nullptr, 0);
}

void RenderCommandD3D11::BindCS(const ComputeShaderDx11& shader) noexcept
{
    m_context->CSSetShader(shader.GetShader().Get(), nullptr, 0);
}

void LS::Win32::RenderCommandD3D11::BindCS(ID3D11ComputeShader* cs) noexcept
{
    m_context->CSSetShader(cs, nullptr, 0);
}

void RenderCommandD3D11::BindHS(const HullShaderDx11& shader) noexcept
{
    m_context->HSSetShader(shader.GetShader().Get(), nullptr, 0);
}

void LS::Win32::RenderCommandD3D11::BindHS(ID3D11HullShader* hs) noexcept
{
    m_context->HSSetShader(hs, nullptr, 0);
}

void RenderCommandD3D11::BindDS(const DomainShaderDx11& shader) noexcept
{
    m_context->DSSetShader(shader.GetShader().Get(), nullptr, 0);
}

void LS::Win32::RenderCommandD3D11::BindDS(ID3D11DomainShader* ds) noexcept
{
    m_context->DSSetShader(ds, nullptr, 0);
}

void LS::Win32::RenderCommandD3D11::BindVSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->VSSetConstantBuffers(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindVSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot)
{
    m_context->VSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void LS::Win32::RenderCommandD3D11::BindVSResource(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->VSSetShaderResources(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindVSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot)
{
    m_context->VSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void LS::Win32::RenderCommandD3D11::BindPSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->PSSetConstantBuffers(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindPSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot)
{
    m_context->PSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void LS::Win32::RenderCommandD3D11::BindPSResource(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->PSSetShaderResources(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindPSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot)
{
    m_context->PSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void LS::Win32::RenderCommandD3D11::BindGSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->GSSetConstantBuffers(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindGSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot)
{
    m_context->GSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void LS::Win32::RenderCommandD3D11::BindGSResource(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->GSSetShaderResources(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindGSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot)
{
    m_context->GSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void LS::Win32::RenderCommandD3D11::BindHSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->HSSetConstantBuffers(slot , 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindHSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot)
{
    m_context->HSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void LS::Win32::RenderCommandD3D11::BindHSResource(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->HSSetShaderResources(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindHSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot)
{
    m_context->HSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void LS::Win32::RenderCommandD3D11::BindDSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->DSSetConstantBuffers(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindDSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot)
{
    m_context->DSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void LS::Win32::RenderCommandD3D11::BindDSResource(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->DSSetShaderResources(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindDSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot)
{
    m_context->DSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void LS::Win32::RenderCommandD3D11::BindCSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->CSSetConstantBuffers(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindCSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot)
{
    m_context->CSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void LS::Win32::RenderCommandD3D11::BindCSResource(ID3D11Buffer* buffer, uint32_t slot)
{
    m_context->CSSetShaderResources(slot, 1u, &buffer);
}

void LS::Win32::RenderCommandD3D11::BindCSResources(std::span<ID3D11Buffer*> buffer, uint32_t slot)
{
    m_context->CSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void RenderCommandD3D11::UpdateTexture(ID3D11Resource* resource, const void* data) noexcept
{
    LS::Platform::Dx11::UpdateSubresource(m_context.Get(), resource, data);
}

void RenderCommandD3D11::UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data) noexcept
{
    LS::Platform::Dx11::UpdateSubresource(m_context.Get(), buffer, data);
}

void RenderCommandD3D11::SetInputLayout(ID3D11InputLayout* il) noexcept
{
    m_context->IASetInputLayout(il);
}

void LS::Win32::RenderCommandD3D11::SetVertexBuffers(std::span<ID3D11Buffer*> vbs, 
    std::span<uint32_t> strides, std::span<uint32_t> offsets, uint32_t startSlot) noexcept
{
    m_context->IASetVertexBuffers(startSlot, (UINT)vbs.size(), vbs.data(), strides.data(), offsets.data());
}

void LS::Win32::RenderCommandD3D11::SetVertexBuffer(ID3D11Buffer* vb, uint32_t stride, uint32_t startSlot, uint32_t offset) noexcept
{
    m_context->IASetVertexBuffers(startSlot, 1, &vb, &stride, &offset);
}

void RenderCommandD3D11::SetIndexBuffer(ID3D11Buffer* ib, uint32_t offset /*= 0*/, DXGI_FORMAT format /*= DXGI_FORMAT_R32_UINT*/) noexcept
{
    m_context->IASetIndexBuffer(ib, format, offset);
}

void RenderCommandD3D11::SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept
{
    m_context->IASetPrimitiveTopology(topology);
}

void RenderCommandD3D11::SetRenderTargets(std::span<ID3D11RenderTargetView*> rtvs, ID3D11DepthStencilView* depthStencilView /*= nullptr*/) noexcept
{
    m_context->OMSetRenderTargets((UINT)rtvs.size(), rtvs.data(), depthStencilView ? depthStencilView : nullptr);
}

void LS::Win32::RenderCommandD3D11::SetRenderTarget(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* depthStencilView) noexcept
{
    m_context->OMSetRenderTargets(1, &rtv, depthStencilView ? depthStencilView : nullptr);
}

void RenderCommandD3D11::SetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef) noexcept
{
    m_context->OMSetDepthStencilState(dss, stencilRef);
}

void RenderCommandD3D11::SetBlendState(ID3D11BlendState* bs, uint32_t sampleMask, std::array<float, 4> blendFactor) noexcept
{
    m_context->OMSetBlendState(bs, blendFactor.data(), sampleMask);
}

void RenderCommandD3D11::SetRasterizerState(ID3D11RasterizerState* rss) noexcept
{
    m_context->RSSetState(rss);
}

void LS::Win32::RenderCommandD3D11::SetViewports(std::span<D3D11_VIEWPORT> viewports) noexcept
{
    m_context->RSSetViewports((UINT)viewports.size(), viewports.data());
}

void RenderCommandD3D11::SetViewport(D3D11_VIEWPORT viewport) noexcept
{
    m_context->RSSetViewports(1u, &viewport);
}

void LS::Win32::RenderCommandD3D11::SetViewport(float width, float height) noexcept
{
    const D3D11_VIEWPORT viewport{.TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = width, .Height = height, .MinDepth = 0.0f, .MaxDepth = 1.0f };
    m_context->RSSetViewports(1, &viewport);
}

void RenderCommandD3D11::Clear(const float* rgbaColor, ID3D11RenderTargetView* rtv) noexcept
{
    m_context->ClearRenderTargetView(rtv, rgbaColor);
}

void RenderCommandD3D11::ClearDepth(ID3D11DepthStencilView* dsv, float depth) noexcept
{
    m_context->ClearDepthStencilView(dsv, (UINT)D3D11_CLEAR_DEPTH, depth, 0);
}

void LS::Win32::RenderCommandD3D11::ClearStencil(ID3D11DepthStencilView* dsv, uint8_t stencil) noexcept
{
    m_context->ClearDepthStencilView(dsv, (UINT)D3D11_CLEAR_STENCIL, 0, stencil);
}

void LS::Win32::RenderCommandD3D11::ClearDepthStencil(ID3D11DepthStencilView* dsv, float depth, uint8_t stencil) noexcept
{
    m_context->ClearDepthStencilView(dsv, (UINT)(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL), depth, stencil);
}

void RenderCommandD3D11::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset) noexcept
{
    m_context->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void RenderCommandD3D11::DrawIndxInstances(uint32_t indexCount, uint32_t instances, uint32_t indexOffset, uint32_t baseOffset, uint32_t instanceOffset) noexcept
{
    m_context->DrawIndexedInstanced(indexCount, instances, indexOffset, baseOffset, instanceOffset);
}

void RenderCommandD3D11::DrawVerts(uint32_t vertexCount, uint32_t vertexOffset) noexcept
{
    m_context->Draw(vertexCount, vertexOffset);
}

void RenderCommandD3D11::DrawVertInstances(uint32_t vertexCount, uint32_t instances, uint32_t vertexOffset, uint32_t instanceOffset) noexcept
{
    m_context->DrawInstanced(vertexCount, instances, vertexOffset, instanceOffset);
}

void RenderCommandD3D11::ClearState() noexcept
{
    m_context->ClearState();
}

void RenderCommandD3D11::FlushCommands() noexcept
{
    m_context->Flush();
}