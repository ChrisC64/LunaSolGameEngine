module;
#include <directx/d3dx12.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

export module D3D12Lib.RendererDX12;
import <cstdint>;
import <unordered_map>;
import <stdexcept>;
import <vector>;

import Engine.LSWindow;
import Engine.LSDevice;
import Engine.LSCamera;
import Engine.EngineCodes;
import Engine.Defines;
import Engine.Logger;

import D3D12Lib.CommandListDx12;
import D3D12Lib.CommandQueueD3D12;
import D3D12Lib.Device;
import D3D12Lib.DescriptorHeapDx12;
import D3D12Lib.FrameBufferDxgi;
import D3D12Lib.FrameDx12;
import D3D12Lib.D3D12Utils.Pipeline;
import Win32.Utils;
import Win32.ComUtils;
import DXGIHelper;

/**
 * @brief A implementation of a forward/immediate renderer for DX12 system.
 * This is a test to implement a hastly put together mode as I also want
 * to create a deferrred renderer system as well to play and understand with.
 *
 * This renderer will contain all the necessary components for users to do the following:
 * 1. Create window to draw on
 * 2. Simplify some of the complexities in "drawing" basic objects/shapes
 * 3. Manage the device's state as an FSM to perform various actions
 */

namespace WRL = Microsoft::WRL;

namespace LS::Platform::Dx12
{
    /**
     * @brief A pair between a view and the resource
     * @tparam T The View type to use
     */
    template<class T>
    struct RVPair
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
        T View;
    };
}

export namespace LS::Platform::Dx12
{
    enum class RendererState
    {
        UNINITIALIZED,// @brief Created, but hasn't been correctly setup
        INITIALIZED,// @brief User has successfully initialized the renderer
        CLEAR_RT, // @brief Clearing the render target's buffer with a color 
        PRESENT_RT, // @brief The drawing is done for one frmae, ready to present 
        WAIT_FOR_FRAME, // @brief Waiting on the GPU fence to draw the next frame
        COMMANDS_IN_FLIGHT, // @brief Commands are in flight to the GPU, executing command buffer
        PAUSED, // @brief Commands 
    };

    class RendererDX12
    {
    public:
        RendererDX12(uint32_t width, uint32_t height, uint32_t frameCount = 2, const LSWindowBase* window = nullptr);
        RendererDX12(uint32_t width, uint32_t height, uint32_t frameCount = 2, HWND window = nullptr);
        ~RendererDX12() = default;

        RendererDX12& operator=(const RendererDX12&) = delete;
        RendererDX12(const RendererDX12&) = delete;

        RendererDX12(RendererDX12&&) = default;
        RendererDX12& operator=(RendererDX12&&) = default;

        // Interface Design -- Sorta //
        // void ClearRenderTarget(const CommandListDx12& cl, const std::array<float, 4>& clearColor);
        //void BeginDraw() const;
        // EndDraw();
        // LoadShader(filepath) -> LS::System::ErrorCode;
        // LoadMesh(filepath) -> LS::System::ErrorCode;

        [[nodiscard]]
        auto CreateCommandList(D3D12_COMMAND_LIST_TYPE type, std::string_view name) const->Nullable<CommandListDx12>;

        void QueueCommand(CommandListDx12* const commandlist);
        void FlushCommands() noexcept;

        [[nodiscard]]
        auto Resize(uint32_t width, uint32_t height) noexcept -> LS::System::ErrorCode;

        void WaitOnNextFrame();
        /**
         * @brief Waits for the given commands to finish. When you call @link RendererDX12::ExecuteCommands
         * the function returns a value that you need to pass back to l
         * @param commandsValue
         */
        void WaitForCommands(uint64_t commandFence) noexcept;

        [[nodiscard]]
        auto ExecuteCommands() -> uint64_t;

        void BeginFrame();
        void PresentFrame();
        /**
         * @brief Assigns and sets the command allocator for this command list. The command list must be closed and finished
         * before another command list can be assigned with this current frame buffer's command allocator.
         * @param commandList The command list to ready for usage.
         */
        void BeginCommandList(CommandListDx12& commandList);
        void BeginCommandList(CommandListDx12& commandList, uint32_t stateId);

        /**
         * @brief Closes the command list, allowing the command allocator in use to be used with another in-flight command list.
         * @param commandList
         */
        void EndCommandList(CommandListDx12& commandList);

        auto BuildPipelineState(Dx12PsoBuilder& builder) -> LS::Nullable<size_t>;

        auto CreateVertexBuffer(const void* pData, size_t size, size_t stride) -> LS::Nullable<size_t>;

        auto SetVertexBuffer(uint32_t vbId, CommandListDx12& commandList, uint32_t slot = 0);

    private: // Members //
        WRL::ComPtr<IDXGIFactory4>              m_pFactory;
        WRL::ComPtr<IDXGIAdapter1>              m_pAdapter;
        RendererState                           m_state = RendererState::UNINITIALIZED;
        FrameBufferDxgi                         m_frameBuffer;
        DeviceD3D12                             m_device;
        CommandQueueDx12                        m_queue;
        FrameContext                            m_frameContext;
        std::unordered_map<size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_pipelines;
        std::unordered_map<size_t, RVPair<D3D12_VERTEX_BUFFER_VIEW>> m_vertexBuffers;
        WRL::ComPtr<ID3D12RootSignature>        m_rootSignature;
    private: // Functions //
        [[nodiscard]]
        auto Initialize(const LSWindowBase* window) noexcept -> LS::System::ErrorCode;

        [[nodiscard]]
        auto Initialize(HWND hwnd) noexcept -> LS::System::ErrorCode;

    public:
        auto GetFrameBufferConst() const noexcept -> const FrameBufferDxgi&
        {
            return m_frameBuffer;
        }

        auto GetFrameBuffer() noexcept -> FrameBufferDxgi&
        {
            return m_frameBuffer;
        }
    };
}

module : private;

using namespace LS::Platform::Dx12;

RendererDX12::RendererDX12(uint32_t width, uint32_t height, uint32_t frameCount, const LSWindowBase* window /*= nullptr*/)
    : m_frameBuffer(frameCount,
        width, height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_SWAP_EFFECT_FLIP_DISCARD,
        DXGI_SCALING_NONE,
        DXGI_ALPHA_MODE_UNSPECIFIED,
        DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT),
    m_device(),
    m_pFactory(),
    m_queue(D3D12_COMMAND_LIST_TYPE_DIRECT),
    m_frameContext(frameCount)
{
    if (const auto ec = Initialize(window); !ec)
    {
        throw std::runtime_error(ec.Message().data());
    }

    m_state = RendererState::INITIALIZED;
}

RendererDX12::RendererDX12(uint32_t width, uint32_t height, uint32_t frameCount, HWND window)
    : m_frameBuffer(frameCount,
        width, height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_SWAP_EFFECT_FLIP_DISCARD,
        DXGI_SCALING_NONE,
        DXGI_ALPHA_MODE_UNSPECIFIED,
        DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT),
    m_device(),
    m_pFactory(),
    m_queue(D3D12_COMMAND_LIST_TYPE_DIRECT),
    m_frameContext(frameCount)
{
    if (const auto ec = Initialize(window); !ec)
    {
        throw std::runtime_error(ec.Message().data());
    }

    m_state = RendererState::INITIALIZED;
}

auto RendererDX12::Initialize(const LSWindowBase* window) noexcept -> LS::System::ErrorCode
{
    HWND hwnd = reinterpret_cast<HWND>(window->GetHandleToWindow());
    return Initialize(hwnd);
}

auto RendererDX12::Initialize(HWND hwnd) noexcept -> LS::System::ErrorCode
{
    UINT flag = 0;
#ifdef _DEBUG
    flag = (UINT)DXGI_CREATE_FACTORY_DEBUG;
#endif
    auto factory = LS::Win32::CreateDXGIFactory2(flag).value_or(nullptr);
    if (!factory)
    {
        return LS::System::ErrorCode("Failed to create DXGI Factory");
    }

    if (HRESULT hr = factory.As(&m_pFactory); FAILED(hr))
    {
        const auto msg = LS::Win32::HrToString(hr);
        return LS::System::ErrorCode(msg);
    }

    auto adapterOptional = LS::Win32::GetHardwareAdapter(m_pFactory.Get());
    if (!adapterOptional)
    {
        return LS::System::ErrorCode("Failed to obtain adapter with hardware requirement.");
    }

    m_pAdapter = adapterOptional.value();

    if (const auto ec = m_device.CreateDevice(m_pAdapter); !ec)
    {
        return ec;
    }

    WRL::ComPtr<ID3D12Device4> device4;
    auto device = m_device.GetDevice();
    auto deviceHr = device.As(&device4);
    if (FAILED(deviceHr))
    {
        return LS::System::ErrorCode("Failed to to create ID3D12Device4 interface.");
    }

    if (const auto ec = m_queue.Initialize(m_device.GetDevice().Get()); !ec)
    {
        return ec;
    }

    if (const auto ec = m_frameBuffer.Initialize(m_pFactory,
        m_queue.GetCommandQueue().Get(),
        hwnd,
        m_device.GetDevice().Get()); !ec)
    {
        return ec;
    }

    //TODO: Make optional or another setting, for now don't allow ALT+ENTER fullscreen swap
    m_pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    for (auto i = 0u; i < m_frameBuffer.GetFrameLatencyCount(); ++i)
    {
        m_frameContext.SetFenceValue(i, 0);
        WRL::ComPtr<ID3D12CommandAllocator> alloc;
        HRESULT hr = device4->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc));
        if (FAILED(hr))
        {
            const std::string msg = LS::Win32::HrToString(hr);
            return LS::System::ErrorCode(msg);
        }
        m_frameContext.SetAllocator(i, alloc);
    }

    m_state = RendererState::INITIALIZED;
    return LS::System::CreateSuccessCode();
}

auto RendererDX12::CreateCommandList(D3D12_COMMAND_LIST_TYPE type, std::string_view name) const -> Nullable<CommandListDx12>
{
    WRL::ComPtr<ID3D12Device4> device4;
    const auto device = m_device.GetDevice();
    const auto deviceHr = device.As(&device4);
    if (FAILED(deviceHr))
    {
        LS::Log::TraceError("Failed to to create ID3D12Device4 interface.");
        return std::nullopt;
    }

    return CommandListDx12(device4.Get(), type, name);
}

void RendererDX12::QueueCommand(CommandListDx12* const commandlist)
{
    m_queue.QueueCommand(commandlist);
}

void RendererDX12::FlushCommands() noexcept
{
    m_queue.Flush();
}

auto RendererDX12::Resize(uint32_t width, uint32_t height) noexcept -> LS::System::ErrorCode
{
    FlushCommands();
    return m_frameBuffer.ResizeFrames(width, height, m_device.GetDevice().Get());
}

void RendererDX12::WaitOnNextFrame()
{
    m_frameBuffer.WaitOnFrameBuffer();
}

void RendererDX12::WaitForCommands(uint64_t commandFence) noexcept
{
    m_queue.WaitForGpu(commandFence);
}

auto RendererDX12::ExecuteCommands() -> uint64_t
{
    const uint64_t fence = m_queue.ExecuteCommandList();
    const auto index = m_frameBuffer.GetCurrentIndex();
    m_frameContext.SetFenceValue(index, fence);
    HRESULT hr = m_frameBuffer.Present();
    if (FAILED(hr))
    {
        const std::string msg = LS::Win32::HrToString(hr);
        throw std::runtime_error(msg.c_str());
    }

    return fence;
}

void RendererDX12::BeginFrame()
{
    // wait for frame buffer to finish presenting if it's not already
    m_frameBuffer.WaitOnFrameBuffer();
    // Get Current Index and Fence
    const auto currFrame = m_frameBuffer.GetCurrentIndex();
    const auto fence = m_frameContext.GetFence(currFrame);
    // Check if Fence is complete, and if it isn't, wait for queue to finish before 
    // we begin work on our next frame
    if (!m_queue.IsFenceComplete(fence))
    {
        // Blocks CPU to wait for GPU 
        m_queue.WaitForGpu(fence);
    }
}

void RendererDX12::PresentFrame()
{
    const uint64_t nextFenceValue = m_queue.ExecuteCommandList();
    const auto index = m_frameBuffer.GetCurrentIndex();
    m_frameContext.SetFenceValue(index, nextFenceValue);
    HRESULT hr = m_frameBuffer.Present();
    if (FAILED(hr))
    {
        const std::string msg = LS::Win32::HrToString(hr);
        throw std::runtime_error(msg.c_str());
    }
}

void RendererDX12::BeginCommandList(CommandListDx12& commandList)
{
    const FrameDx12& frame = m_frameBuffer.GetCurrentFrame();

    WRL::ComPtr<ID3D12CommandAllocator>& alloc = m_frameContext.GetAllocator(m_frameBuffer.GetCurrentIndex());
    LS::Utils::ThrowIfFailed(alloc->Reset(), "The command list failed to reset.");
    commandList.BeginFrame(frame, alloc);
}

void RendererDX12::BeginCommandList(CommandListDx12& commandList, uint32_t stateId)
{
    const FrameDx12& frame = m_frameBuffer.GetCurrentFrame();

    WRL::ComPtr<ID3D12CommandAllocator>& alloc = m_frameContext.GetAllocator(m_frameBuffer.GetCurrentIndex());
    LS::Utils::ThrowIfFailed(alloc->Reset(), "The command list failed to reset.");
    commandList.Begin(alloc);
    const auto state = m_pipelines[stateId];
    commandList.SetPipelineState(state.Get());
    commandList.SetGraphicsRootSignature(m_rootSignature.Get());
    commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    const auto dimensions = m_frameBuffer.GetDimensions();
    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(dimensions.x), static_cast<float>(dimensions.y));
    CD3DX12_RECT scissorRect(0, 0, dimensions.x, dimensions.y);
    commandList.SetScissorRects(1, &scissorRect);
    commandList.SetViewports(1, &viewport);
    commandList.SetRenderTarget(frame);
}

void RendererDX12::EndCommandList(CommandListDx12& commandList)
{
    commandList.EndFrame();
}

auto RendererDX12::BuildPipelineState(Dx12PsoBuilder& builder) -> LS::Nullable<size_t>
{
    const auto pso = builder.BuildPSO(m_device.GetDevice().Get());
    if (!pso)
        return std::nullopt;

    const auto id = m_pipelines.size() + 1;
    m_pipelines[id] = pso;
    m_rootSignature = builder.GetRootSignature();

    return id;
}
//TODO: Should probably request a command list from the user, but for now will tie to same queue
// or we could create our own internal upload queue. We'll see...
auto RendererDX12::CreateVertexBuffer(const void* pData, size_t size, size_t stride) -> LS::Nullable<size_t>
{
    Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    LS::Utils::ThrowIfFailed(m_device.GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer)), "Failed to create committed resource.");

    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    HRESULT hr = buffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    if (FAILED(hr))
    {
#ifdef _DEBUG
        throw std::runtime_error("Failed to map buffer for writing.");
#else
        return std::nullopt;
#endif
    }

    memcpy(pVertexDataBegin, pData, size);
    buffer->Unmap(0, nullptr);
    D3D12_VERTEX_BUFFER_VIEW view;
    view.BufferLocation = buffer->GetGPUVirtualAddress();
    view.StrideInBytes = stride;
    view.SizeInBytes = size;

    const auto id = m_vertexBuffers.size() + 1;
    m_vertexBuffers[id] = RVPair<D3D12_VERTEX_BUFFER_VIEW>{ .Resource = buffer, .View = view };
    return id;
}

auto RendererDX12::SetVertexBuffer(uint32_t vbId, CommandListDx12& commandList, uint32_t slot /*= 0*/)
{
    if (!m_vertexBuffers.contains(vbId))
        return;
    const auto vb = m_vertexBuffers[vbId];

    commandList.SetVertexBuffers(slot, 1, &vb.View);
}
