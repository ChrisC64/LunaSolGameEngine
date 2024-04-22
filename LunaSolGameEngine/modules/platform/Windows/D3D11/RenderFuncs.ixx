module;
#include <cassert>
#include <array>
#include <span>
#include <d3d11_4.h>
#include <wrl/client.h>
#include <limits>
#include <dxgi.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "engine/EngineDefines.h"
#include "engine/EngineLogDefines.h"

#pragma comment(lib, "d3d11")
#define NOMINMAX
#undef max
#undef min
export module D3D11.RenderFuncD3D11;
import Util.MSUtils;
import LSEDataLib;
import Engine.LSDevice;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    // CLEAR //
    constexpr void ClearRT(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRTView,
        std::array<float, 4> color) noexcept
    {
        assert(pContext);
        assert(pRTView);
        if (!pContext || !pRTView)
            return;

        pContext->ClearRenderTargetView(pRTView, color.data());
    }

    constexpr void ClearDS(ID3D11DeviceContext* pContext, ID3D11DepthStencilView* pDSView, uint32_t flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        float depth = 1.0f, uint8_t stencil = 0) noexcept
    {
        assert(pContext);
        assert(pDSView);
        if (!pContext || !pDSView)
            return;
        pContext->ClearDepthStencilView(pDSView, flags, depth, stencil);
    }

    [[nodiscard]]
    constexpr auto CreateRenderTargetView(ID3D11Device* pDevice, ID3D11Resource* pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc = nullptr) -> ID3D11RenderTargetView*
    {
        assert(pDevice);
        assert(pResource);

        ID3D11RenderTargetView* pRTView;
        HRESULT hr = pDevice->CreateRenderTargetView(pResource, rtvDesc, &pRTView);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create render target view");
        return pRTView;
    }

    [[nodiscard]]
    constexpr auto CreateRenderTargetView1(ID3D11Device3* pDevice, ID3D11Resource* pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc = nullptr) -> ID3D11RenderTargetView1*
    {
        assert(pDevice);
        assert(pResource);

        ID3D11RenderTargetView1* pRTView;
        HRESULT hr = pDevice->CreateRenderTargetView1(pResource, rtvDesc, &pRTView);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create render target view");
        return pRTView;
    }

    [[nodiscard]]
    inline auto CreateRenderTargetViewFromSwapChain(WRL::ComPtr<ID3D11Device> pDevice, WRL::ComPtr<IDXGISwapChain> pSwapChain) noexcept -> Nullable<WRL::ComPtr<ID3D11RenderTargetView>>
    {
        assert(pSwapChain);
        assert(pDevice);
        WRL::ComPtr<ID3D11Texture2D> backBuffer;
        WRL::ComPtr<ID3D11RenderTargetView> rtv;
        HRESULT hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr))
        {
            LS_LOG_ERROR(L"Failed to get back buffer from swap chain");
            return std::nullopt;
        }

        D3D11_TEXTURE2D_DESC swapDesc;
        backBuffer->GetDesc(&swapDesc);
        CD3D11_RENDER_TARGET_VIEW_DESC cdesc(backBuffer.Get(), D3D11_RTV_DIMENSION_TEXTURE2D, swapDesc.Format);

        hr = pDevice->CreateRenderTargetView(backBuffer.Get(), &cdesc, &rtv);
        if (FAILED(hr))
        {
            LS_LOG_ERROR(L"Failed to create render target view");
            return std::nullopt;
        }
        return rtv;
    }

    [[nodiscard]]
    inline auto CreateDepthStencilViewFromSwapChain(WRL::ComPtr<ID3D11Device> pDevice, WRL::ComPtr<IDXGISwapChain> pSwapChain, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT) noexcept -> Nullable<WRL::ComPtr<ID3D11DepthStencilView>>
    {
        assert(pDevice);
        assert(pSwapChain);
        WRL::ComPtr<ID3D11Texture2D> backBuffer;
        auto result = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(result))
        {
            LS_LOG_ERROR(L"Failed to obtain back buffer for CreateDepthStencilViewFromSwapChain");
            return std::nullopt;
        }

        D3D11_TEXTURE2D_DESC depthBufferDesc{};
        backBuffer->GetDesc(&depthBufferDesc);
        depthBufferDesc.Format = format;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        WRL::ComPtr<ID3D11Texture2D> depthBuffer;
        WRL::ComPtr<ID3D11DepthStencilView> depthStencil;
        
        result = pDevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);
        if (FAILED(result))
        {
            LS_LOG_ERROR(L"Failed to create the texture resource in CreateDepthStencilViewFromSwapChain");
            return std::nullopt;
        }

        result = pDevice->CreateDepthStencilView(depthBuffer.Get(), nullptr, &depthStencil);
        if (FAILED(result))
        {
            LS_LOG_ERROR(L"Failed to create the depth stencil view CreateDepthStencilViewFromSwapChain");
            return std::nullopt;
        }
        return depthStencil;
    }

    constexpr void SetRenderTarget(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRTView, ID3D11DepthStencilView* pDSView, uint32_t numViews = 1) noexcept
    {
        assert(pContext);
        assert(pRTView);
        if (!pContext || !pRTView)
            return;
        pContext->OMSetRenderTargets(numViews, &pRTView, pDSView);
    }

    constexpr void SetRenderTarget(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* const* pRTView, ID3D11DepthStencilView* pDSView, uint32_t numViews = 1) noexcept
    {
        assert(pContext);
        assert(pRTView);
        if (!pContext || !pRTView)
            return;
        pContext->OMSetRenderTargets(numViews, pRTView, pDSView);
    }

    constexpr void UnbindRenderTarget(ID3D11DeviceContext* pContext, ID3D11DepthStencilView* pDSView = nullptr)
    {
        assert(pContext);
        pContext->OMSetRenderTargets(0, nullptr, pDSView);
    }

    // DRAW CALLS //

    /**
     * @brief Draws the number of vertices supplied to the vertex buffer
     * @param pContext The context to issue the draw call
     * @param vertexCount Number of vertices to draw
     * @param vertexOffset Offset from the buffer to start from
    */
    constexpr void Draw(ID3D11DeviceContext* pContext, uint32_t vertexCount, uint32_t vertexOffset = 0) noexcept
    {
        assert(pContext);
        pContext->Draw(vertexCount, vertexOffset);
    }

    /**
     * @brief Draws a number of instances of an object
     * @param pContext the D3D11 Device Context
     * @param bufferSize the size of the buffer that is being used
     * @param instances number of instances to draw
     * @param indexOffset an offset to the index location data in the buffer
     * @param baseOffset the offset to each index of the next starting point in the vertex buffer
     * @param instanceOffset an offset for the instance data in the indexed buffer
    */
    constexpr void DrawInstances(ID3D11DeviceContext* pContext, uint32_t indexBufferSize, uint32_t instances,
        uint32_t indexOffset, uint32_t baseOffset, uint32_t instanceOffset) noexcept
    {
        assert(pContext);
        pContext->DrawIndexedInstanced(indexBufferSize, instances, indexOffset, baseOffset, instanceOffset);
    }

    /**
     * @brief Draws a number of instances from the set vertex buffer
     * @param pContext the context to use in this draw call
     * @param vertexCount number of vertices to drwa
     * @param instances the number of instances
     * @param vertexOffset the offset in the vertexBuffer to start reading from
     * @param instanceOffset the offset value of each instance in the instanced buffer
     */
    constexpr void DrawInstances(ID3D11DeviceContext* pContext, uint32_t vertexCount, uint32_t instances,
        uint32_t vertexOffset, uint32_t instanceOffset) noexcept
    {
        assert(pContext);
        pContext->DrawInstanced(vertexCount, instances, vertexOffset, instanceOffset);
    }

    /**
     * @brief Draws a vertex buffer with the associated index buffer provided
     * @param pContext the context to draw with
     * @param indexCount the number of indices to read from
     * @param indexOffset the offset from the index buffer to start at
     * @param vertexOffset the number applied to the index count when reading (for non-interleaved types usually)
     */
    constexpr void DrawIndexed(ID3D11DeviceContext* pContext, uint32_t indexCount, uint32_t indexOffset = 0,
        uint32_t vertexOffset = 0) noexcept
    {
        assert(pContext);
        pContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
    }

    // END DRAW CALLS //

    // PRESENT CALLS //

    constexpr void Present(IDXGISwapChain* pSwapchain, uint32_t syncInterval = 0, uint32_t flags = 0) noexcept
    {
        assert(pSwapchain);
        pSwapchain->Present(syncInterval, flags);
    }

    constexpr void Present1(IDXGISwapChain1* pSwapchain, uint32_t syncInterval = 0, uint32_t flags = 0, const DXGI_PRESENT_PARAMETERS* params = nullptr) noexcept
    {
        assert(pSwapchain);
        constexpr DXGI_PRESENT_PARAMETERS presentParams{
            .DirtyRectsCount = 0,
            .pDirtyRects = nullptr,
            .pScrollRect = nullptr,
            .pScrollOffset = nullptr
        };
        if (!params)
        {
            params = &presentParams;
        }
        pSwapchain->Present1(syncInterval, flags, params);
    }

    // END PRESENT CALLS //

    // Shader Binders //
    constexpr void BindVS(ID3D11DeviceContext* pContext, ID3D11VertexShader* shader) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->VSSetShader(shader, nullptr, 0);
    }

    constexpr void BindVS(ID3D11DeviceContext* pContext, ID3D11VertexShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->VSSetShader(shader, &classInstance, numInstances);
    }

    constexpr void BindVSConstantBuffers(ID3D11DeviceContext* pContext, uint32_t startSlot, std::span<ID3D11Buffer*> buffers)
    {
        assert(pContext);
        assert(!buffers.empty());

        if (buffers.empty() && buffers.size() <= std::numeric_limits<uint32_t>::max())
            return;

        pContext->VSSetConstantBuffers(startSlot, static_cast<uint32_t>(buffers.size()), buffers.data());
    }
    
    constexpr void BindVSConstantBuffer(ID3D11DeviceContext* pContext, uint32_t slot, ID3D11Buffer* buffer)
    {
        assert(pContext);
        if (!pContext)
            return;

        pContext->VSSetConstantBuffers(slot, 1u, &buffer);
    }

    constexpr void BindVSResources(ID3D11DeviceContext* pContext, std::span<ID3D11ShaderResourceView*> views, uint32_t startSlot = 0)
    {
        assert(pContext);

        pContext->VSSetShaderResources(startSlot, (UINT)views.size(), views.data());
    }

    constexpr void BindVSResource(ID3D11DeviceContext* pContext, ID3D11ShaderResourceView* view, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->VSSetShaderResources(slot, 1u, &view);
    }

    constexpr void BindVSSampler(ID3D11DeviceContext* pContext, ID3D11SamplerState* sampler, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->VSSetSamplers(slot, 1u, &sampler);
    }
    
    constexpr void BindVSSamplers(ID3D11DeviceContext* pContext, std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->VSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
    }

    constexpr void BindPS(ID3D11DeviceContext* pContext, ID3D11PixelShader* shader) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->PSSetShader(shader, nullptr, 0);
    }

    constexpr void BindPS(ID3D11DeviceContext* pContext, ID3D11PixelShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->PSSetShader(shader, &classInstance, numInstances);
    }

    constexpr void BindPSConstantBuffers(ID3D11DeviceContext* pContext, uint32_t startSlot, std::span<ID3D11Buffer*> buffers)
    {
        assert(pContext);
        assert(!buffers.empty());
        assert(buffers.size() <= std::numeric_limits<uint32_t>::max());
        if (buffers.empty() && buffers.size() <= std::numeric_limits<uint32_t>::max())
            return;

        pContext->PSSetConstantBuffers(startSlot, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    constexpr void BindPSResources(ID3D11DeviceContext* pContext, std::span<ID3D11ShaderResourceView*> views, uint32_t startSlot = 0)
    {
        assert(pContext);

        pContext->PSSetShaderResources(startSlot, (UINT)views.size(), views.data());
    }

    constexpr void BindPSResource(ID3D11DeviceContext* pContext, ID3D11ShaderResourceView* view, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->PSSetShaderResources(slot, 1u, &view);
    }

    constexpr void BindPSSampler(ID3D11DeviceContext* pContext, ID3D11SamplerState* sampler, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->PSSetSamplers(slot, 1u, &sampler);
    }

    constexpr void BindPSSamplers(ID3D11DeviceContext* pContext, std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->PSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
    }
    
    constexpr void BindGS(ID3D11DeviceContext* pContext, ID3D11GeometryShader* shader) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->GSSetShader(shader, nullptr, 0);
    }

    constexpr void BindGS(ID3D11DeviceContext* pContext, ID3D11GeometryShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->GSSetShader(shader, &classInstance, numInstances);
    }

    constexpr void BindGSConstantBuffers(ID3D11DeviceContext* pContext, uint32_t startSlot, std::span<ID3D11Buffer*> buffers)
    {
        assert(pContext);
        assert(!buffers.empty());
        assert(buffers.size() <= std::numeric_limits<uint32_t>::max());
        if (buffers.empty() && buffers.size() <= std::numeric_limits<uint32_t>::max())
            return;

        pContext->GSSetConstantBuffers(startSlot, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    constexpr void BindGSResources(ID3D11DeviceContext* pContext, std::span<ID3D11ShaderResourceView*> views, uint32_t startSlot = 0)
    {
        assert(pContext);

        pContext->GSSetShaderResources(startSlot, (UINT)views.size(), views.data());
    }

    constexpr void BindGSResource(ID3D11DeviceContext* pContext, ID3D11ShaderResourceView* view, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->GSSetShaderResources(slot, 1u, &view);
    }

    constexpr void BindGSSampler(ID3D11DeviceContext* pContext, ID3D11SamplerState* sampler, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->GSSetSamplers(slot, 1u, &sampler);
    }

    constexpr void BindGSSamplers(ID3D11DeviceContext* pContext, std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->GSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
    }

    constexpr void BindCS(ID3D11DeviceContext* pContext, ID3D11ComputeShader* shader) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->CSSetShader(shader, nullptr, 0);
    }

    constexpr void BindCS(ID3D11DeviceContext* pContext, ID3D11ComputeShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->CSSetShader(shader, &classInstance, numInstances);
    }

    constexpr void BindCSResources(ID3D11DeviceContext* pContext, std::span<ID3D11ShaderResourceView*> views, uint32_t startSlot = 0)
    {
        assert(pContext);

        pContext->CSSetShaderResources(startSlot, (UINT)views.size(), views.data());
    }

    constexpr void BindCSResource(ID3D11DeviceContext* pContext, ID3D11ShaderResourceView* view, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->CSSetShaderResources(slot, 1u, &view);
    }

    constexpr void BindCSConstantBuffers(ID3D11DeviceContext* pContext, uint32_t startSlot, std::span<ID3D11Buffer*> buffers)
    {
        assert(pContext);
        assert(!buffers.empty());
        assert(buffers.size() <= std::numeric_limits<uint32_t>::max());
        if (buffers.empty() && buffers.size() <= std::numeric_limits<uint32_t>::max())
            return;

        pContext->CSSetConstantBuffers(startSlot, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    constexpr void BindCSSampler(ID3D11DeviceContext* pContext, ID3D11SamplerState* sampler, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->CSSetSamplers(slot, 1u, &sampler);
    }

    constexpr void BindCSSamplers(ID3D11DeviceContext* pContext, std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->CSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
    }

    constexpr void BindHS(ID3D11DeviceContext* pContext, ID3D11HullShader* shader) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->HSSetShader(shader, nullptr, 0);
    }

    constexpr void BindHS(ID3D11DeviceContext* pContext, ID3D11HullShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->HSSetShader(shader, &classInstance, numInstances);
    }

    constexpr void BindHSConstantBuffers(ID3D11DeviceContext* pContext, uint32_t startSlot, std::span<ID3D11Buffer*> buffers)
    {
        assert(pContext);
        assert(!buffers.empty());
        assert(buffers.size() <= std::numeric_limits<uint32_t>::max());
        if (buffers.empty() && buffers.size() <= std::numeric_limits<uint32_t>::max())
            return;

        pContext->HSSetConstantBuffers(startSlot, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    constexpr void BindHSResources(ID3D11DeviceContext* pContext, std::span<ID3D11ShaderResourceView*> views, uint32_t startSlot = 0)
    {
        assert(pContext);

        pContext->HSSetShaderResources(startSlot, (UINT)views.size(), views.data());
    }

    constexpr void BindHSResource(ID3D11DeviceContext* pContext, ID3D11ShaderResourceView* view, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->HSSetShaderResources(slot, 1u, &view);
    }

    constexpr void BindHSSampler(ID3D11DeviceContext* pContext, ID3D11SamplerState* sampler, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->HSSetSamplers(slot, 1u, &sampler);
    }

    constexpr void BindHSSamplers(ID3D11DeviceContext* pContext, std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->HSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
    }

    constexpr void BindDS(ID3D11DeviceContext* pContext, ID3D11DomainShader* shader) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->DSSetShader(shader, nullptr, 0);
    }

    constexpr void BindDS(ID3D11DeviceContext* pContext, ID3D11DomainShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances) noexcept
    {
        assert(pContext);
        assert(shader);
        pContext->DSSetShader(shader, &classInstance, numInstances);
    }

    constexpr void BindDSConstantBuffers(ID3D11DeviceContext* pContext, uint32_t startSlot, std::span<ID3D11Buffer*> buffers)
    {
        assert(pContext);
        assert(!buffers.empty());
        assert(buffers.size() <= std::numeric_limits<uint32_t>::max());
        if (buffers.empty() && buffers.size() <= std::numeric_limits<uint32_t>::max())
            return;

        pContext->DSSetConstantBuffers(startSlot, static_cast<uint32_t>(buffers.size()), buffers.data());
    }

    constexpr void BindDSResources(ID3D11DeviceContext* pContext, std::span<ID3D11ShaderResourceView*> views, uint32_t startSlot = 0)
    {
        assert(pContext);

        pContext->DSSetShaderResources(startSlot, (UINT)views.size(), views.data());
    }

    constexpr void BindDSResource(ID3D11DeviceContext* pContext, ID3D11ShaderResourceView* view, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->DSSetShaderResources(slot, 1u, &view);
    }

    constexpr void BindDSSampler(ID3D11DeviceContext* pContext, ID3D11SamplerState* sampler, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->DSSetSamplers(slot, 1u, &sampler);
    }

    constexpr void BindDSSamplers(ID3D11DeviceContext* pContext, std::span<ID3D11SamplerState*> samplers, uint32_t slot = 0)
    {
        assert(pContext);

        pContext->DSSetSamplers(slot, (UINT)samplers.size(), samplers.data());
    }

    // Input Assembly //
    constexpr void SetTopology(ID3D11DeviceContext* pContext, D3D11_PRIMITIVE_TOPOLOGY topology) noexcept
    {
        assert(pContext);
        pContext->IASetPrimitiveTopology(topology);
    }

    // Blend State //
    constexpr void SetBlendState(ID3D11DeviceContext* pContext, ID3D11BlendState* pBlendState,
        const std::array<float, 4>& blendFactor = { 1.f, 1.f, 1.f, 1.f }, uint32_t sampleMask = 0xffffffff) noexcept
    {
        assert(pContext);
        pContext->OMSetBlendState(pBlendState, blendFactor.data(), sampleMask);
    }

    // Depth Stencil //
    constexpr void SetDepthStencilState(ID3D11DeviceContext* pContext, ID3D11DepthStencilState* dsState, uint32_t stencilRef)
    {
        assert(pContext);
        assert(dsState);
        pContext->OMSetDepthStencilState(dsState, stencilRef);
    }

    // Rasterizer State //
    constexpr void SetRasterizerState(ID3D11DeviceContext* pContext, ID3D11RasterizerState* state)
    {
        assert(pContext);
        assert(state);
        pContext->RSSetState(state);
    }

    constexpr void SetViewport(ID3D11DeviceContext* pContext, float width, float height, float topX = 0.0f, float topY = 0.0f) noexcept
    {
        assert(pContext);
        CD3D11_VIEWPORT viewport(topX, topY, width, height);
        pContext->RSSetViewports(1, &viewport);
    }

    constexpr void SetViewports(ID3D11DeviceContext* pContext, std::span<D3D11_VIEWPORT> viewports) noexcept
    {
        assert(pContext);
        uint32_t count = 0u;
        if (viewports.empty())
            return;
        count = static_cast<uint32_t>(viewports.size());
        pContext->RSSetViewports(count, &viewports.front());
    }

    constexpr void SetInputlayout(ID3D11DeviceContext* pContext, ID3D11InputLayout* layout) noexcept
    {
        assert(pContext);
        if (!pContext)
            return;
        pContext->IASetInputLayout(layout);
    }

    constexpr void SetVertexBuffers(ID3D11DeviceContext* pContext, std::span<ID3D11Buffer*> pBuffer, uint32_t startSlot,
        uint32_t stride, uint32_t offset = 0u) noexcept
    {
        assert(pContext);
        if (!pContext)
            return;

        pContext->IASetVertexBuffers(startSlot, static_cast<UINT>(pBuffer.size()), pBuffer.data(), &stride, &offset);
    }

    constexpr void SetVertexBuffers(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, uint32_t numBuffers, uint32_t startSlot,
        uint32_t stride, uint32_t offset = 0u) noexcept
    {
        assert(pContext);
        if (!pContext)
            return;

        pContext->IASetVertexBuffers(startSlot, numBuffers, &pBuffer, &stride, &offset);
    }

    constexpr void SetVertexBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, uint32_t startSlot, uint32_t stride, uint32_t offset = 0u) noexcept
    {
        assert(pContext);
        if (!pContext)
            return;

        pContext->IASetVertexBuffers(startSlot, 1, &pBuffer, &stride, &offset);
    }

    constexpr void SetIndexBuffer(ID3D11DeviceContext* pContext, ID3D11Buffer* pBuffer, uint32_t offset = 0, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT)
    {
        assert(pContext);
        if (!pContext)
            return;

        pContext->IASetIndexBuffer(pBuffer, format, offset);
    }

    constexpr auto BuildSwapchainDesc1(uint32_t frameCount, uint32_t width, uint32_t height, PIXEL_COLOR_FORMAT pixelFormat) noexcept -> DXGI_SWAP_CHAIN_DESC1
    {
        DXGI_SWAP_CHAIN_DESC1 swDesc1{};
        swDesc1.BufferCount = frameCount;
        swDesc1.Height = height;
        swDesc1.Width = width;
        using enum PIXEL_COLOR_FORMAT;
        DXGI_FORMAT format;
        switch (pixelFormat)
        {
        case RGBA8_UNORM:
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        case BGRA8_UNORM:
            format = DXGI_FORMAT_B8G8R8A8_UNORM;
            break;
        case RGBA16_UNORM:
            format = DXGI_FORMAT_R16G16B16A16_UNORM;
            break;
        case BGRA16_UNORM:
            format = DXGI_FORMAT_R16G16B16A16_UNORM;
            break;
        default:
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        }

        swDesc1.Format = format;
        swDesc1.Stereo = false;
        swDesc1.SampleDesc.Count = 1;
        swDesc1.SampleDesc.Quality = 0;
        swDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swDesc1.Scaling = DXGI_SCALING_NONE;
        swDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swDesc1.Flags = 0;
        return swDesc1;
    }
}