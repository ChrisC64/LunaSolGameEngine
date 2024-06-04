module;
#include <d3d11_4.h>
#include <wrl/client.h>
#include <directxtk/CommonStates.h>

#ifdef OPAQUE // Windows GDI leaking 
#undef OPAQUE
#endif
export module D3D11.RenderCommandD3D11;
import <span>;
import <array>;
import Engine.Defines;
import D3D11.HelperStates;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    enum class COMMAND_MODE
    {
        IMMEDIATE,
        DEFERRED
    };

    enum class CULL_METHOD
    {
        CULL_NONE,
        CULL_BACKFACE,
        CULL_BACKFACE_CC,
        CULL_FRONTFACE,
        CULL_FRONTFACE_CC,
        WIREFRAME,
        WIREFRAME_CC
    };

    enum class DEPTH_BUFFER_MODE
    {
        NO_DEPTH,
        DEFAULT,
        READ,
        REVERSE_Z,
        READ_REVERSE_Z
    };

    enum class BLEND_MODE
    {
        OPAQUE,
        ADDITIVE,
        ALPHA_BLEND,
        NON_PRE_MULTIPLIED
    };

    class RenderCommandD3D11
    {
    private:
        WRL::ComPtr<ID3D11DeviceContext> m_context;
        COMMAND_MODE m_mode;
        SharedRef<DirectX::CommonStates> m_commonStates;

    public:
        RenderCommandD3D11() = default;
        RenderCommandD3D11(ID3D11Device* pDevice, COMMAND_MODE mode);
        ~RenderCommandD3D11() = default;

        // Bind Shaders to Pipeline //
        void BindVS(ID3D11VertexShader* vs) const noexcept;
        void BindPS(ID3D11PixelShader* ps) const noexcept;
        void BindGS(ID3D11GeometryShader* gs) const noexcept;
        void BindCS(ID3D11ComputeShader* cs) const noexcept;
        void BindHS(ID3D11HullShader* hs) const noexcept;
        void BindDS(ID3D11DomainShader* ds) const noexcept;

        // Bind Buffers to Shader Stages Stages //
        void BindVSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0) const;
        void BindVSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0) const;
        void BindVSResource(ID3D11ShaderResourceView* buffer, uint32_t slot = 0) const;
        void BindVSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot = 0) const;
        void BindVSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0) const;
        void BindVSSampler(ID3D11SamplerState* sampler, uint32_t slot = 0) const;

        void BindPSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0) const;
        void BindPSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0) const;
        void BindPSResource(ID3D11ShaderResourceView* buffer, uint32_t slot = 0) const;
        void BindPSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot = 0) const;
        void BindPSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0) const;
        void BindPSSampler(ID3D11SamplerState* sampler, uint32_t slot = 0) const;

        void BindGSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0) const;
        void BindGSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0) const;
        void BindGSResource(ID3D11ShaderResourceView* buffer, uint32_t slot = 0) const;
        void BindGSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot = 0) const;
        void BindGSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0) const;
        void BindGSSampler(ID3D11SamplerState* sampler, uint32_t slot = 0) const;

        void BindHSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0) const;
        void BindHSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0) const;
        void BindHSResource(ID3D11ShaderResourceView* buffer, uint32_t slot = 0) const;
        void BindHSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot = 0) const;
        void BindHSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0) const;
        void BindHSSampler(ID3D11SamplerState* sampler, uint32_t slot = 0) const;

        void BindDSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0) const;
        void BindDSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0) const;
        void BindDSResource(ID3D11ShaderResourceView* buffer, uint32_t slot = 0) const;
        void BindDSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot = 0) const;
        void BindDSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0) const;
        void BindDSSampler(ID3D11SamplerState* sampler, uint32_t slot = 0) const;

        void BindCSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot = 0) const;
        void BindCSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot = 0) const;
        void BindCSResource(ID3D11ShaderResourceView* buffer, uint32_t slot = 0) const;
        void BindCSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot = 0) const;
        void BindCSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0) const;
        void BindCSSampler(ID3D11SamplerState* sampler, uint32_t slot = 0) const;

        // Bind Commands for Resources for Shaders //
        void UpdateTexture(ID3D11Resource* resource, const void* data) const noexcept;
        void UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data) const noexcept;

        // Input Assembly Functions //
        void SetInputLayout(ID3D11InputLayout* il) const noexcept;
        void SetVertexBuffers(std::span<ID3D11Buffer*> vbs, std::span<uint32_t> strides, std::span<uint32_t> offsets, uint32_t startSlot = 0) const noexcept;
        void SetVertexBuffer(ID3D11Buffer* vb, uint32_t stride, uint32_t startSlot = 0, uint32_t offset = 0) const noexcept;
        void SetIndexBuffer(ID3D11Buffer* ib, uint32_t offset = 0, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT) const noexcept;
        void SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY topology) const noexcept;

        // Output Merger Functions //
        void SetRenderTargets(std::span<ID3D11RenderTargetView*> rtvs, ID3D11DepthStencilView* depthStencilView = nullptr) const noexcept;
        void SetRenderTarget(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* depthStencilView = nullptr) const noexcept;
        void SetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef = 0) const noexcept;
        void SetBlendState(ID3D11BlendState* bs, uint32_t sampleMask = 0xffffffff, std::array<float, 4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f }) const noexcept;

        // Set Input For this Draw State //
        void SetRasterizerState(ID3D11RasterizerState* rss) const noexcept;
        void SetViewports(std::span<D3D11_VIEWPORT> viewports) const noexcept;
        void SetViewport(D3D11_VIEWPORT viewports) const noexcept;
        void SetViewport(float width, float height) const noexcept;

        // Draw Commands //
        void Clear(const std::array<float, 4>& rgbaColor, ID3D11RenderTargetView* rtv) const noexcept;
        void ClearDepth(ID3D11DepthStencilView* dsv, float depth = 1.0f) const noexcept;
        void ClearStencil(ID3D11DepthStencilView* dsv, uint8_t stencil = 0) const noexcept;
        void ClearDepthStencil(ID3D11DepthStencilView* dsv, float depth = 1.0f, uint8_t stencil = 0) const noexcept;
        void DrawIndexed(uint32_t indexCount, uint32_t indexOffset = 0,
            uint32_t vertexOffset = 0) const noexcept;
        void DrawIndxInstances(uint32_t indexCount, uint32_t instances, uint32_t indexOffset = 0,
            uint32_t baseOffset = 0, uint32_t instanceOffset = 0) const noexcept;
        void DrawVerts(uint32_t vertexCount, uint32_t vertexOffset = 0) const noexcept;
        void DrawVertInstances(uint32_t vertexCount, uint32_t instances,
            uint32_t vertexOffset = 0, uint32_t instanceOffset = 0) const noexcept;

        // State Operations //
        // @brief Resets to default state
        void ClearState() const noexcept;
        void EnableWireframe() const noexcept;
        void SetCullMethod(CULL_METHOD method) const noexcept;
        void SetBlendMode(BLEND_MODE mode, uint32_t sampleMask = 0xffffffff, const std::array<float,4> blendFactor = {1.0f, 1.0f, 1.0f, 1.0f} ) const noexcept;
        void SetDepthBufferMode(DEPTH_BUFFER_MODE mode, uint32_t stencilRef = 0xFF) const noexcept;
        // @brief Expunge all commands recorded up to this point
        void FlushCommands() const noexcept;

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

        auto GetCommandList() const -> LS::Nullable<ID3D11CommandList*>
        {
            if (m_mode == COMMAND_MODE::IMMEDIATE)
                return {};
            ID3D11CommandList* pCommList;
            const auto hr = m_context->FinishCommandList(false, &pCommList);
            if (FAILED(hr))
            {
                return {};
            }

            return pCommList;
        }

    };
}

module : private;

import Win32.ComUtils;
import D3D11.MemoryHelper;

using namespace LS::Win32;

RenderCommandD3D11::RenderCommandD3D11(ID3D11Device* pDevice, COMMAND_MODE mode) : m_mode(mode)
{
    if (mode == COMMAND_MODE::IMMEDIATE)
    {
        pDevice->GetImmediateContext(&m_context);
    }
    else
    {
        LS::Utils::ThrowIfFailed(pDevice->CreateDeferredContext(0, &m_context));
    }

    m_commonStates = std::make_shared<DirectX::CommonStates>(pDevice);
}

void RenderCommandD3D11::BindVS(ID3D11VertexShader* vs) const noexcept
{
    m_context->VSSetShader(vs, nullptr, 0);
}

void RenderCommandD3D11::BindPS(ID3D11PixelShader* ps) const noexcept
{
    m_context->PSSetShader(ps, nullptr, 0);
}

void RenderCommandD3D11::BindGS(ID3D11GeometryShader* gs) const noexcept
{
    m_context->GSSetShader(gs, nullptr, 0);
}

void RenderCommandD3D11::BindCS(ID3D11ComputeShader* cs) const noexcept
{
    m_context->CSSetShader(cs, nullptr, 0);
}

void RenderCommandD3D11::BindHS(ID3D11HullShader* hs) const noexcept
{
    m_context->HSSetShader(hs, nullptr, 0);
}

void RenderCommandD3D11::BindDS(ID3D11DomainShader* ds) const noexcept
{
    m_context->DSSetShader(ds, nullptr, 0);
}

void RenderCommandD3D11::BindVSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot) const
{
    m_context->VSSetConstantBuffers(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindVSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot) const
{
    m_context->VSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void RenderCommandD3D11::BindVSResource(ID3D11ShaderResourceView* buffer, uint32_t slot) const
{
    m_context->VSSetShaderResources(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindVSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot) const
{
    m_context->VSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void RenderCommandD3D11::BindVSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot) const
{
    m_context->VSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
}

void RenderCommandD3D11::BindVSSampler(ID3D11SamplerState* sampler, uint32_t slot) const
{
    m_context->VSSetSamplers(slot, 1u, &sampler);
}

void RenderCommandD3D11::BindPSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot) const
{
    m_context->PSSetConstantBuffers(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindPSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot) const
{
    m_context->PSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void RenderCommandD3D11::BindPSResource(ID3D11ShaderResourceView* buffer, uint32_t slot) const
{
    m_context->PSSetShaderResources(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindPSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot) const
{
    m_context->PSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void RenderCommandD3D11::BindPSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot) const
{
    m_context->PSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
}

void RenderCommandD3D11::BindPSSampler(ID3D11SamplerState* sampler, uint32_t slot) const
{
    m_context->PSSetSamplers(slot, 1u, &sampler);
}

void RenderCommandD3D11::BindGSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot) const
{
    m_context->GSSetConstantBuffers(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindGSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot) const
{
    m_context->GSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void RenderCommandD3D11::BindGSResource(ID3D11ShaderResourceView* buffer, uint32_t slot) const
{
    m_context->GSSetShaderResources(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindGSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot) const
{
    m_context->GSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void RenderCommandD3D11::BindGSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot) const
{
    m_context->GSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
}

void RenderCommandD3D11::BindGSSampler(ID3D11SamplerState* sampler, uint32_t slot) const
{
    m_context->GSSetSamplers(slot, 1u, &sampler);
}

void RenderCommandD3D11::BindHSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot) const
{
    m_context->HSSetConstantBuffers(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindHSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot) const
{
    m_context->HSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void RenderCommandD3D11::BindHSResource(ID3D11ShaderResourceView* buffer, uint32_t slot) const
{
    m_context->HSSetShaderResources(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindHSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot) const
{
    m_context->HSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void RenderCommandD3D11::BindHSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot) const
{
    m_context->HSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
}

void RenderCommandD3D11::BindHSSampler(ID3D11SamplerState* sampler, uint32_t slot) const
{
    m_context->HSSetSamplers(slot, 1u, &sampler);
}

void RenderCommandD3D11::BindDSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot) const
{
    m_context->DSSetConstantBuffers(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindDSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot) const
{
    m_context->DSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void RenderCommandD3D11::BindDSResource(ID3D11ShaderResourceView* buffer, uint32_t slot) const
{
    m_context->DSSetShaderResources(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindDSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot) const
{
    m_context->DSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void RenderCommandD3D11::BindDSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot) const
{
    m_context->DSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
}

void RenderCommandD3D11::BindDSSampler(ID3D11SamplerState* sampler, uint32_t slot) const
{
    m_context->DSSetSamplers(slot, 1u, &sampler);
}

void RenderCommandD3D11::BindCSConstantBuffer(ID3D11Buffer* buffer, uint32_t slot) const
{
    m_context->CSSetConstantBuffers(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindCSConstantBuffers(std::span<ID3D11Buffer*> buffers, uint32_t startSlot) const
{
    m_context->CSSetConstantBuffers(startSlot, (UINT)buffers.size(), buffers.data());
}

void RenderCommandD3D11::BindCSResource(ID3D11ShaderResourceView* buffer, uint32_t slot) const
{
    m_context->CSSetShaderResources(slot, 1u, &buffer);
}

void RenderCommandD3D11::BindCSResources(std::span<ID3D11ShaderResourceView*> buffer, uint32_t slot) const
{
    m_context->CSSetShaderResources(slot, (UINT)buffer.size(), buffer.data());
}

void RenderCommandD3D11::BindCSSamplers(std::span<ID3D11SamplerState*> samplers, uint32_t slot) const
{
    m_context->CSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
}

void RenderCommandD3D11::BindCSSampler(ID3D11SamplerState* sampler, uint32_t slot) const
{
    m_context->CSSetSamplers(slot, 1u, &sampler);
}

void RenderCommandD3D11::UpdateTexture(ID3D11Resource* resource, const void* data) const noexcept
{
    LS::Platform::Dx11::UpdateSubresource(m_context.Get(), resource, data);
}

void RenderCommandD3D11::UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data) const noexcept
{
    LS::Platform::Dx11::UpdateSubresource(m_context.Get(), buffer, data);
}

void RenderCommandD3D11::SetInputLayout(ID3D11InputLayout* il) const noexcept
{
    m_context->IASetInputLayout(il);
}

void RenderCommandD3D11::SetVertexBuffers(std::span<ID3D11Buffer*> vbs,
    std::span<uint32_t> strides, std::span<uint32_t> offsets, uint32_t startSlot) const noexcept
{
    m_context->IASetVertexBuffers(startSlot, (UINT)vbs.size(), vbs.data(), strides.data(), offsets.data());
}

void RenderCommandD3D11::SetVertexBuffer(ID3D11Buffer* vb, uint32_t stride, uint32_t startSlot, uint32_t offset) const noexcept
{
    m_context->IASetVertexBuffers(startSlot, 1, &vb, &stride, &offset);
}

void RenderCommandD3D11::SetIndexBuffer(ID3D11Buffer* ib, uint32_t offset /*= 0*/, DXGI_FORMAT format /*= DXGI_FORMAT_R32_UINT*/) const noexcept
{
    m_context->IASetIndexBuffer(ib, format, offset);
}

void RenderCommandD3D11::SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY topology) const noexcept
{
    m_context->IASetPrimitiveTopology(topology);
}

void RenderCommandD3D11::SetRenderTargets(std::span<ID3D11RenderTargetView*> rtvs, ID3D11DepthStencilView* depthStencilView /*= nullptr*/) const noexcept
{
    m_context->OMSetRenderTargets((UINT)rtvs.size(), rtvs.data(), depthStencilView ? depthStencilView : nullptr);
}

void RenderCommandD3D11::SetRenderTarget(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* depthStencilView) const noexcept
{
    m_context->OMSetRenderTargets(1, &rtv, depthStencilView ? depthStencilView : nullptr);
}

void RenderCommandD3D11::SetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef) const noexcept
{
    m_context->OMSetDepthStencilState(dss, stencilRef);
}

void RenderCommandD3D11::SetBlendState(ID3D11BlendState* bs, uint32_t sampleMask, std::array<float, 4> blendFactor) const noexcept
{
    m_context->OMSetBlendState(bs, blendFactor.data(), sampleMask);
}

void RenderCommandD3D11::SetRasterizerState(ID3D11RasterizerState* rss) const noexcept
{
    m_context->RSSetState(rss);
}

void RenderCommandD3D11::SetViewports(std::span<D3D11_VIEWPORT> viewports) const noexcept
{
    m_context->RSSetViewports((UINT)viewports.size(), viewports.data());
}

void RenderCommandD3D11::SetViewport(D3D11_VIEWPORT viewport) const noexcept
{
    m_context->RSSetViewports(1u, &viewport);
}

void RenderCommandD3D11::SetViewport(float width, float height) const noexcept
{
    const D3D11_VIEWPORT viewport{ .TopLeftX = 0.0f, .TopLeftY = 0.0f, .Width = width, .Height = height, .MinDepth = 0.0f, .MaxDepth = 1.0f };
    m_context->RSSetViewports(1, &viewport);
}

void RenderCommandD3D11::Clear(const std::array<float, 4>& rgbaColor, ID3D11RenderTargetView* rtv) const noexcept
{
    m_context->ClearRenderTargetView(rtv, rgbaColor.data());
}

void RenderCommandD3D11::ClearDepth(ID3D11DepthStencilView* dsv, float depth) const noexcept
{
    m_context->ClearDepthStencilView(dsv, (UINT)D3D11_CLEAR_DEPTH, depth, 0);
}

void RenderCommandD3D11::ClearStencil(ID3D11DepthStencilView* dsv, uint8_t stencil) const noexcept
{
    m_context->ClearDepthStencilView(dsv, (UINT)D3D11_CLEAR_STENCIL, 0, stencil);
}

void RenderCommandD3D11::ClearDepthStencil(ID3D11DepthStencilView* dsv, float depth, uint8_t stencil) const noexcept
{
    m_context->ClearDepthStencilView(dsv, (UINT)(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL), depth, stencil);
}

void RenderCommandD3D11::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset) const noexcept
{
    m_context->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void RenderCommandD3D11::DrawIndxInstances(uint32_t indexCount, uint32_t instances, 
    uint32_t indexOffset, uint32_t baseOffset, uint32_t instanceOffset) const noexcept
{
    m_context->DrawIndexedInstanced(indexCount, instances, indexOffset, baseOffset, instanceOffset);
}

void RenderCommandD3D11::DrawVerts(uint32_t vertexCount, uint32_t vertexOffset) const noexcept
{
    m_context->Draw(vertexCount, vertexOffset);
}

void RenderCommandD3D11::DrawVertInstances(uint32_t vertexCount, uint32_t instances, 
    uint32_t vertexOffset, uint32_t instanceOffset) const noexcept
{
    m_context->DrawInstanced(vertexCount, instances, vertexOffset, instanceOffset);
}

void RenderCommandD3D11::ClearState() const noexcept
{
    m_context->ClearState();
}

void LS::Win32::RenderCommandD3D11::EnableWireframe() const noexcept
{
    m_context->RSSetState(m_commonStates->Wireframe());
}

void LS::Win32::RenderCommandD3D11::SetCullMethod(CULL_METHOD method) const noexcept
{
    using enum CULL_METHOD;
    switch (method)
    {
    case CULL_NONE:
        m_context->RSSetState(m_commonStates->CullNone());
        break;
    case CULL_BACKFACE:
        m_context->RSSetState(m_commonStates->CullClockwise());
        break;
    case CULL_BACKFACE_CC:
        m_context->RSSetState(m_commonStates->CullCounterClockwise());
        break;
    case CULL_FRONTFACE:
        m_context->RSSetState(m_commonStates->CullClockwise());
        break;
    case CULL_FRONTFACE_CC:
        m_context->RSSetState(m_commonStates->CullCounterClockwise());
        break;
    case WIREFRAME:
        m_context->RSSetState(m_commonStates->Wireframe());
        break;
    case WIREFRAME_CC:
        m_context->RSSetState(m_commonStates->Wireframe());
        break;
    }
}

void LS::Win32::RenderCommandD3D11::SetBlendMode(BLEND_MODE mode, 
    uint32_t sampleMask, const std::array<float, 4> blendFactor) const noexcept
{
    using enum LS::Win32::BLEND_MODE;
    switch (mode)
    {
    case OPAQUE:
        LS::Win32::SetBlendState(m_context.Get(), m_commonStates->Opaque(), sampleMask, blendFactor);
        break;
    case ADDITIVE:
        LS::Win32::SetBlendState(m_context.Get(), m_commonStates->Additive(), sampleMask, blendFactor);
        break;
    case ALPHA_BLEND:
        LS::Win32::SetBlendState(m_context.Get(), m_commonStates->AlphaBlend(), sampleMask, blendFactor);
        break;
    case NON_PRE_MULTIPLIED:
        LS::Win32::SetBlendState(m_context.Get(), m_commonStates->NonPremultiplied(), sampleMask, blendFactor);
        break;
    default:
        break;
    }
}

void LS::Win32::RenderCommandD3D11::SetDepthBufferMode(DEPTH_BUFFER_MODE mode, uint32_t stencilRef) const noexcept
{
    using enum LS::Win32::DEPTH_BUFFER_MODE;

    switch (mode)
    {
    case NO_DEPTH:
        LS::Win32::SetDepthStencilState(m_context.Get(), m_commonStates->DepthNone(), stencilRef);
        break;
    case DEFAULT:
        LS::Win32::SetDepthStencilState(m_context.Get(), m_commonStates->DepthDefault(), stencilRef);
        break;
    case READ:
        LS::Win32::SetDepthStencilState(m_context.Get(), m_commonStates->DepthRead(), stencilRef);
        break;
    case REVERSE_Z:
        LS::Win32::SetDepthStencilState(m_context.Get(), m_commonStates->DepthReverseZ(), stencilRef);
        break;
    case READ_REVERSE_Z:
        LS::Win32::SetDepthStencilState(m_context.Get(), m_commonStates->DepthReadReverseZ(), stencilRef);
        break;
    default:
        break;
    }
}

void RenderCommandD3D11::FlushCommands() const noexcept
{
    m_context->Flush();
}