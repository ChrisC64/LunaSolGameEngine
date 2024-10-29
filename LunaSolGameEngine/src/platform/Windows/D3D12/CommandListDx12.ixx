module;
#include <cstdint>
#include <array>
#include <string_view>
#include <string>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>

export module D3D12Lib.CommandListDx12;
import <cassert>;
import Engine.EngineCodes;
import D3D12Lib.FrameBufferDxgi;
import D3D12Lib.FrameDx12;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandListDx12
    {
    public:
        CommandListDx12() = default;
        CommandListDx12(D3D12_COMMAND_LIST_TYPE type, std::string_view name);
        CommandListDx12(ID3D12Device4* pDevice, D3D12_COMMAND_LIST_TYPE type, std::string_view name);
        ~CommandListDx12() = default;
        // Copy //
        CommandListDx12(const CommandListDx12&) = delete;
        CommandListDx12& operator=(const CommandListDx12&) = delete;
        // Move // 
        CommandListDx12(CommandListDx12&&) = default;
        CommandListDx12& operator=(CommandListDx12&&) = default;

        auto Initialize(ID3D12Device4* pDevice) noexcept -> LS::System::ErrorCode;

        auto GetCommandList() const noexcept -> WRL::ComPtr<ID3D12GraphicsCommandList>
        {
            return m_pCommandList;
        }

        auto GetCommandListConst() const noexcept -> const WRL::ComPtr<ID3D12GraphicsCommandList>&
        {
            return m_pCommandList;
        }

        auto GetName() const noexcept -> std::string_view
        {
            return m_name;
        }

        void Begin(const WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator);
        void BeginFrame(const FrameDx12& frame, const WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator);
        void BeginFrame(const FrameBufferDxgi& frameBuffer, const WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator);
        void End() noexcept;
        void EndFrame() noexcept;
        void Clear(std::array<float, 4> clearColor) noexcept;
        void ClearDepthStencil(const D3D12_CPU_DESCRIPTOR_HANDLE dsv, float clearValue = 1.0f, uint8_t stencilValue = 255);
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
        void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle = nullptr) noexcept;
        void SetPipelineState(ID3D12PipelineState* state) noexcept;
        void SetGraphicsRootSignature(ID3D12RootSignature* rootSignature) noexcept;
        void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) noexcept;
        void SetVertexBuffers(uint32_t startSlot, uint32_t numViews, const D3D12_VERTEX_BUFFER_VIEW* views) noexcept;
        void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* indexBuffer) noexcept;
        void SetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports) noexcept;
        void SetScissorRects(uint32_t numRects, const D3D12_RECT* rects) noexcept;
        void SetGraphicsRoot32BitConstant(uint32_t rootParamIndx, uint32_t srcData, uint32_t destOffsetIn32BitValues) noexcept;
        void SetGraphicsRoot32BitConstants(uint32_t rootParamIndx, uint32_t num32BitValueToSet, const void* pData, uint32_t destOffsetIn32BitValues) noexcept;
        void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation = 0u, int32_t  baseVertexLocation = 0u, uint32_t startInstanceLocation = 0u) noexcept;

        void SetRenderTarget(const FrameDx12& frame, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle = nullptr) noexcept;
        void FinishFrame() noexcept;

    private:
        D3D12_COMMAND_LIST_TYPE m_type;
        WRL::ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
        std::string m_name;
        const FrameDx12* m_currentFrame = nullptr;
    };
}

module : private;

import <cstdint>;
import <cassert>;
import <format>;
import Win32.ComUtils;
import Engine.EngineCodes;
import Win32.Utils;

using namespace LS::Platform::Dx12;

CommandListDx12::CommandListDx12(D3D12_COMMAND_LIST_TYPE type, std::string_view name)
    : m_type(type),
    m_pCommandList(nullptr),
    m_name(name)
{
}

CommandListDx12::CommandListDx12(ID3D12Device4* pDevice, D3D12_COMMAND_LIST_TYPE type, std::string_view name)
    : m_type(type),
    m_pCommandList(nullptr),
    m_name(name)
{
    pDevice->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_pCommandList));
    assert(m_pCommandList && "Failed to create command list");
}

auto CommandListDx12::Initialize(ID3D12Device4* pDevice) noexcept -> LS::System::ErrorCode
{
    auto hr = pDevice->CreateCommandList1(0, m_type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_pCommandList));
    assert(m_pCommandList && "Failed to create command list");

    if (FAILED(hr))
    {
        const auto msg = LS::Win32::HrToString(hr);
        return LS::System::CreateFailCode(std::format("Failed to create command list. Error {}", msg));
    }
    return LS::System::CreateSuccessCode();
}

void CommandListDx12::Begin(const WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator)
{
    LS::Utils::ThrowIfFailed(m_pCommandList->Reset(commandAllocator.Get(), nullptr), std::format("Failed to reset command allocator: {}", m_name.c_str()));
}

void CommandListDx12::BeginFrame(const FrameDx12& frame, const WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator)
{
    Begin(commandAllocator);
    SetRenderTarget(frame);
}

void CommandListDx12::BeginFrame(const FrameBufferDxgi& frameBuffer, const WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator)
{
    const FrameDx12& frame = frameBuffer.GetCurrentFrame();
    Begin(commandAllocator);
    SetRenderTarget(frame);
}

void CommandListDx12::End() noexcept
{
    m_pCommandList->Close();
    m_currentFrame = nullptr;
}

void LS::Platform::Dx12::CommandListDx12::EndFrame() noexcept
{
    FinishFrame();
    End();
}

void CommandListDx12::Clear(std::array<float, 4> clearColor) noexcept
{
    assert(m_currentFrame && "No frame was set, did you forget to BeginFrame() or SetRenderTarget()");
    m_pCommandList->ClearRenderTargetView(*(m_currentFrame->GetDescriptorHandle()), clearColor.data(), 0, nullptr);
}

void CommandListDx12::ClearDepthStencil(const D3D12_CPU_DESCRIPTOR_HANDLE dsv, float clearValue, uint8_t stencilValue)
{
    m_pCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearValue, stencilValue, 0, nullptr);
}

void CommandListDx12::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after);
    m_pCommandList->ResourceBarrier(1, &barrier);
}

void CommandListDx12::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle /*= nullptr*/) noexcept
{
    m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, depthStencilHandle);
}

void CommandListDx12::SetPipelineState(ID3D12PipelineState* state) noexcept
{
    m_pCommandList->SetPipelineState(state);
}

void CommandListDx12::SetGraphicsRootSignature(ID3D12RootSignature* rootSignature) noexcept
{
    m_pCommandList->SetGraphicsRootSignature(rootSignature);
}

void CommandListDx12::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) noexcept
{
    m_pCommandList->IASetPrimitiveTopology(topology);
}

void CommandListDx12::SetVertexBuffers(uint32_t startSlot, uint32_t numViews, const D3D12_VERTEX_BUFFER_VIEW* views) noexcept
{
    m_pCommandList->IASetVertexBuffers(startSlot, numViews, views);
}

void CommandListDx12::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* indexBuffer) noexcept
{
    m_pCommandList->IASetIndexBuffer(indexBuffer);
}

void CommandListDx12::SetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports) noexcept
{
    m_pCommandList->RSSetViewports(numViewports, viewports);
}

void CommandListDx12::SetScissorRects(uint32_t numRects, const D3D12_RECT* rects) noexcept
{
    m_pCommandList->RSSetScissorRects(numRects, rects);
}

void CommandListDx12::SetGraphicsRoot32BitConstant(uint32_t rootParamIndx, uint32_t srcData, uint32_t destOffsetIn32BitValues) noexcept
{
    m_pCommandList->SetGraphicsRoot32BitConstant(rootParamIndx, srcData, destOffsetIn32BitValues);
}

void CommandListDx12::SetGraphicsRoot32BitConstants(uint32_t rootParamIndx, uint32_t num32BitValueToSet, const void* pData, uint32_t destOffsetIn32BitValues) noexcept
{
    m_pCommandList->SetGraphicsRoot32BitConstants(rootParamIndx, num32BitValueToSet, pData, destOffsetIn32BitValues);
}

void CommandListDx12::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation /*= 0*/, int32_t baseVertexLocation /*= 0*/, uint32_t startInstanceLocation /*= 0*/) noexcept
{
    m_pCommandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandListDx12::SetRenderTarget(const FrameDx12& frame, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle /*= nullptr*/) noexcept
{
    m_currentFrame = &frame;
    TransitionResource(m_currentFrame->GetFrame().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_pCommandList->OMSetRenderTargets(1, m_currentFrame->GetDescriptorHandle(), FALSE, depthStencilHandle);
}

void LS::Platform::Dx12::CommandListDx12::FinishFrame() noexcept
{
    assert(m_currentFrame && "No frame was set, did you forget to call SetRenderTarget");
    TransitionResource(m_currentFrame->GetFrame().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}