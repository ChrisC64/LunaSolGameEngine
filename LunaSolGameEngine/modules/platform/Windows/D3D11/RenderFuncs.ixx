module;
#include <cassert>
#include <array>
#include <span>
#include <d3d11_4.h>
#include <wrl/client.h>
#include <limits>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma comment(lib, "d3d11")
export module D3D11.RenderFuncD3D11;
import Util.MSUtils;
import LSData;
import Engine.LSDevice;

namespace WRL = Microsoft::WRL;

#define NOMINMAX
#undef max
#undef min
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
    constexpr ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Device* pDevice, ID3D11Resource* pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc = nullptr) noexcept
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
    constexpr ID3D11RenderTargetView1* CreateRenderTargetView1(ID3D11Device3* pDevice, ID3D11Resource* pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc = nullptr) noexcept
    {
        assert(pDevice);
        assert(pResource);

        ID3D11RenderTargetView1* pRTView;
        HRESULT hr = pDevice->CreateRenderTargetView1(pResource, rtvDesc, &pRTView);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create render target view");
        return pRTView;
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
    constexpr void DrawIndexed(ID3D11DeviceContext* pContext, uint32_t indexCount, uint32_t indexOffset,
        uint32_t vertexOffset) noexcept
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
        //assert(buffers.size() <= std::numeric_limits<uint32_t>::max());
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

    constexpr void BindCSConstantBuffers(ID3D11DeviceContext* pContext, uint32_t startSlot, std::span<ID3D11Buffer*> buffers)
    {
        assert(pContext);
        assert(!buffers.empty());
        assert(buffers.size() <= std::numeric_limits<uint32_t>::max());
        if (buffers.empty() && buffers.size() <= std::numeric_limits<uint32_t>::max())
            return;

        pContext->CSSetConstantBuffers(startSlot, static_cast<uint32_t>(buffers.size()), buffers.data());
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

    constexpr auto BuildSwapchainDesc1(const LS::LSSwapchainInfo& info) noexcept -> DXGI_SWAP_CHAIN_DESC1
    {
        DXGI_SWAP_CHAIN_DESC1 swDesc1{};
        swDesc1.BufferCount = info.BufferSize;
        swDesc1.Height = info.Height;
        swDesc1.Width = info.Width;
        using enum PIXEL_COLOR_FORMAT;
        DXGI_FORMAT format;
        switch (info.PixelFormat)
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