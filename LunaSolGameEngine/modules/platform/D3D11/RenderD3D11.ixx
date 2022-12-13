module;
#include "LSEFramework.h"

export module D3D11.RenderD3D11;
import Util.MSUtils;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    // CLEAR //
    constexpr void ClearRT(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRTView, std::array<float, 4>& color)
    {
        assert(pContext);
        assert(pRTView);
        if (!pContext || !pRTView)
            return;

        pContext->ClearRenderTargetView(pRTView, color.data());
    }

    constexpr void ClearDS(ID3D11DeviceContext* pContext, ID3D11DepthStencilView* pDSView, uint32_t flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        float depth = 1.0f, uint8_t stencil = 0)
    {
        assert(pContext);
        assert(pDSView);
        if (!pContext || !pDSView)
            return;
        pContext->ClearDepthStencilView(pDSView, flags, depth, stencil);
    }

    [[nodiscard]]
    constexpr ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Device* pDevice, ID3D11Resource* pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc = nullptr)
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
        const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc = nullptr)
    {
        assert(pDevice);
        assert(pResource);

        ID3D11RenderTargetView1* pRTView;
        HRESULT hr = pDevice->CreateRenderTargetView1(pResource, rtvDesc, &pRTView);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create render target view");
        return pRTView;
    }

    constexpr void SetRenderTarget(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRTView, ID3D11DepthStencilView* pDSView, uint32_t numViews = 1)
    {
        assert(pContext);
        assert(pRTView);
        if (!pContext || !pDSView || !pRTView)
            return;
        pContext->OMSetRenderTargets(numViews, &pRTView, pDSView);
    }

    // DRAW CALLS //

    /**
     * @brief Draws the number of vertices supplied to the vertex buffer
     * @param pContext The context to issue the draw call
     * @param vertexCount Number of vertices to draw 
     * @param vertexOffset Offset from the buffer to start from
    */
    constexpr void Draw(ID3D11DeviceContext* pContext, uint32_t vertexCount, uint32_t vertexOffset = 0)
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
        uint32_t indexOffset, uint32_t baseOffset, uint32_t instanceOffset)
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
        uint32_t vertexOffset, uint32_t instanceOffset)
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
        uint32_t vertexOffset)
    {
        assert(pContext);
        pContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
    }

    // PRESENT CALLS //

    constexpr void Present(IDXGISwapChain* pSwapchain, uint32_t syncInterval = 0, uint32_t flags = 0)
    {
        assert(pSwapchain);
        pSwapchain->Present(syncInterval, flags);
    }
    
    constexpr void Present1(IDXGISwapChain1* pSwapchain, uint32_t syncInterval = 0, uint32_t flags = 0, const DXGI_PRESENT_PARAMETERS* params = nullptr)
    {
        assert(pSwapchain);
        pSwapchain->Present1(syncInterval, flags, params);
    }

    // Shader Binders //
    constexpr void BindVS(ID3D11DeviceContext* pContext, ID3D11VertexShader* shader)
    {
        assert(pContext);
        assert(shader);
        pContext->VSSetShader(shader, nullptr, 0);
    }
    
    constexpr void BindVS(ID3D11DeviceContext* pContext, ID3D11VertexShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances)
    {
        assert(pContext);
        assert(shader);
        pContext->VSSetShader(shader, &classInstance, numInstances);
    }
    
    constexpr void BindPS(ID3D11DeviceContext* pContext, ID3D11PixelShader* shader)
    {
        assert(pContext);
        assert(shader);
        pContext->PSSetShader(shader, nullptr, 0);
    }
    
    constexpr void BindPS(ID3D11DeviceContext* pContext, ID3D11PixelShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances)
    {
        assert(pContext);
        assert(shader);
        pContext->PSSetShader(shader, &classInstance, numInstances);
    }
    
    constexpr void BindGS(ID3D11DeviceContext* pContext, ID3D11GeometryShader* shader)
    {
        assert(pContext);
        assert(shader);
        pContext->GSSetShader(shader, nullptr, 0);
    }
    
    constexpr void BindGS(ID3D11DeviceContext* pContext, ID3D11GeometryShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances)
    {
        assert(pContext);
        assert(shader);
        pContext->GSSetShader(shader, &classInstance, numInstances);
    }
    
    constexpr void BindCS(ID3D11DeviceContext* pContext, ID3D11ComputeShader* shader)
    {
        assert(pContext);
        assert(shader);
        pContext->CSSetShader(shader, nullptr, 0);
    }
    
    constexpr void BindCS(ID3D11DeviceContext* pContext, ID3D11ComputeShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances)
    {
        assert(pContext);
        assert(shader);
        pContext->CSSetShader(shader, &classInstance, numInstances);
    }
    
    constexpr void BindHS(ID3D11DeviceContext* pContext, ID3D11HullShader* shader)
    {
        assert(pContext);
        assert(shader);
        pContext->HSSetShader(shader, nullptr, 0);
    }
    
    constexpr void BindHS(ID3D11DeviceContext* pContext, ID3D11HullShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances)
    {
        assert(pContext);
        assert(shader);
        pContext->HSSetShader(shader, &classInstance, numInstances);
    }
    
    constexpr void BindDS(ID3D11DeviceContext* pContext, ID3D11DomainShader* shader)
    {
        assert(pContext);
        assert(shader);
        pContext->DSSetShader(shader, nullptr, 0);
    }
    
    constexpr void BindDS(ID3D11DeviceContext* pContext, ID3D11DomainShader* shader, ID3D11ClassInstance* classInstance, uint32_t numInstances)
    {
        assert(pContext);
        assert(shader);
        pContext->DSSetShader(shader, &classInstance, numInstances);
    }

    // Input Assembly //
    constexpr void SetTopology(ID3D11DeviceContext* pContext, D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        assert(pContext);
        pContext->IASetPrimitiveTopology(topology);
    }

    // Rasterizer State //
    inline void SetViewport(ID3D11DeviceContext* pContext, float width, float height, float topX = 0.0f, float topY = 0.0f)
    {
        assert(pContext);
        CD3D11_VIEWPORT viewport(topX, topY, width, height);
        pContext->RSSetViewports(1, &viewport);
    }

    inline void SetViewports(ID3D11DeviceContext* pContext, std::span<D3D11_VIEWPORT> viewports)
    {
        assert(pContext);
        uint32_t count = 0u;
        if (viewports.empty())
            return;
        count = static_cast<uint32_t>(viewports.size());
        pContext->RSSetViewports(count, &viewports.front());
    }

    inline void SetInputlayout(ID3D11DeviceContext* pContext, ID3D11InputLayout* layout)
    {
        assert(pContext);
        if (!pContext)
            return;
        pContext->IASetInputLayout(layout);
    }
}