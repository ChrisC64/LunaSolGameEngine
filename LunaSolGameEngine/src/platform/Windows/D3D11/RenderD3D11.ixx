module;
#include <wrl/client.h>
#include <d3d11_4.h>
export module D3D11.RenderD3D11;

import <cstdint>;
import <array>;
import <span>;
import <filesystem>;
import Engine.LSWindow;
import Engine.LSDevice;
import Engine.EngineCodes;
import Engine.Defines;
import D3D11.PipelineFactory;
import D3D11.Device;
import D3D11.Utils;
import D3D11.PipelineFactory;
import D3D11.RenderFuncD3D11;
import D3D11.MemoryHelper;
import Win32.ComUtils;
import DirectXCommon;
import DXGISwapChain;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    enum class SHADER_RESOURCE_TYPE
    {
        CBUFFER,
        SAMPLER,
        TEXTURE,
        UAV
    };

    enum class DEPTH_STENCIL_MODE
    {
        NONE,
        DEPTH_ONLY,
        STENCIL_ONLY,
        DEFAULT
    };

    class RenderCommandD3D11;

    class RenderD3D11
    {
    public:
        RenderD3D11(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow);
        ~RenderD3D11();

        // Interface Design (WIP)
        void SetViewport(uint32_t width, uint32_t height, uint32_t topLeft = 0u, uint32_t topRight = 0u) const noexcept;
        void Clear(const std::array<float, 4>& clearColor, DEPTH_STENCIL_MODE mode = LS::Win32::DEPTH_STENCIL_MODE::NONE,
            float depth = 1.0f, uint8_t stencil = 0) noexcept;
        void Draw() noexcept;

        void Resize(uint32_t width, uint32_t height) noexcept;
        void SetFps(uint32_t fps) noexcept;

        // Class Members // 
        [[nodiscard]]
        auto Initialize() noexcept -> LS::System::ErrorCode;
        void SetPipeline(const PipelineStateDX11* state) noexcept;
        void Shutdown() noexcept;
        void AttachToWindow(LS::LSWindowBase* window) noexcept;

        [[nodiscard]]
        auto GetDevice() const noexcept -> ID3D11Device*;
        [[nodiscard]]
        auto GetDeviceCom() const noexcept -> WRL::ComPtr<ID3D11Device>;
        [[nodiscard]]
        auto GetSwapChainCom() const noexcept -> WRL::ComPtr<IDXGISwapChain1>;
        [[nodiscard]]
        auto GetDeviceContextCom() const noexcept -> WRL::ComPtr<ID3D11DeviceContext>;

        [[nodiscard]]
        auto CreateVertexShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11VertexShader>;
        [[nodiscard]]
        auto CreatePixelShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11PixelShader>;
        [[nodiscard]]
        auto CreateGeometryShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11GeometryShader>;
        [[nodiscard]]
        auto CreateDomainShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11DomainShader>;
        [[nodiscard]]
        auto CreateHullShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11HullShader>;
        [[nodiscard]]
        auto CreateComputeShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11ComputeShader>;
        [[nodiscard]]
        auto CreateImmediateCommand() const noexcept -> RenderCommandD3D11;
        [[nodiscard]]
        auto CreateDeferredCommand() const noexcept -> RenderCommandD3D11;
        
        [[nodiscard]]
        auto InitializeSwapchain() noexcept -> LS::System::ErrorCode;
        /**
         * @brief Creates an input layout from the given compiled bytecode. This will not work if it was not compiled first.
         *
         * @param compiledByteCode The compiled byte code of the shader
         * @return Nullable<WRL::ComPtr<ID3D11InputLayout>> A nullable object of WRL::ComPtr<ID3D11InputLayout>>
         */
        [[nodiscard]]
        auto BuildInputLayout(std::span<std::byte> compiledByteCode) -> Nullable<WRL::ComPtr<ID3D11InputLayout>>;

        void ExecuteRenderCommand(const RenderCommandD3D11& command) noexcept;

        auto GetFrameIndex() const noexcept -> uint64_t
        {
            return m_frameIndex;
        }

    protected:
        LS::LSWindowBase*                       m_window;

    private:
        DeviceD3D11                             m_device;
        LS::LSDeviceSettings                    m_settings;

        WRL::ComPtr<ID3D11RenderTargetView>     m_renderTarget;
        WRL::ComPtr<ID3D11DepthStencilView>     m_dsv;
        WRL::ComPtr<ID3D11Texture2D>            m_dsBuffer;
        WRL::ComPtr<ID3D11DeviceContext>        m_context;
        DXGISwapChain                           m_swapchain;
        DEPTH_STENCIL_MODE                      m_dsMode;
        uint64_t                                m_frameIndex;

        [[nodiscard]]
        bool InitializeRtvAndDsv() noexcept;
    };
}

module : private;

import <cassert>;
import "engine/EngineLogDefines.h";
import D3D11.RenderCommandD3D11;
import Engine.Defines;

using namespace LS::Win32;

bool IsCompiled(std::span<std::byte> data)
{
    assert(data.size() > 4 && "The data is not valid, cannot perform check.");
    if (data.size() < 4)
        return false;
    const char* flag = "DXBC";
    const std::string check{ (char)(data[0]), (char)data[1], (char)data[2], (char)data[3] };
    return check == flag;
}

RenderD3D11::RenderD3D11(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow) : m_settings(settings),
m_window(pWindow)
{
}

RenderD3D11::~RenderD3D11()
{
    Shutdown();
}

void LS::Win32::RenderD3D11::SetViewport(uint32_t width, uint32_t height, uint32_t topLeft /*= 0u*/, uint32_t topRight /*= 0u*/) const noexcept
{
    const CD3D11_VIEWPORT viewport((float)topLeft, (float)topRight, (float)width, (float)height);
    m_context->RSSetViewports(1, &viewport);
}

void RenderD3D11::Clear(const std::array<float, 4>& clearColor, LS::Win32::DEPTH_STENCIL_MODE mode, float depth, uint8_t stencil) noexcept
{
    using enum LS::Win32::DEPTH_STENCIL_MODE;
    switch (mode)
    {
    case NONE:
        LS::Win32::SetRenderTarget(m_context.Get(), m_renderTarget.Get());
        break;
    case DEPTH_ONLY:
        LS::Win32::SetRenderTarget(m_context.Get(), m_renderTarget.Get(), m_dsv.Get());
        LS::Win32::ClearDS(m_context.Get(), m_dsv.Get(), depth, stencil, D3D11_CLEAR_DEPTH);
        break;
    case STENCIL_ONLY:
        LS::Win32::SetRenderTarget(m_context.Get(), m_renderTarget.Get(), m_dsv.Get());
        LS::Win32::ClearDS(m_context.Get(), m_dsv.Get(), depth, stencil, D3D11_CLEAR_STENCIL);
        break;
    case DEFAULT:
        LS::Win32::SetRenderTarget(m_context.Get(), m_renderTarget.Get(), m_dsv.Get());
        LS::Win32::ClearDS(m_context.Get(), m_dsv.Get(), depth, stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
        break;
    default:
        LS::Win32::SetRenderTarget(m_context.Get(), m_renderTarget.Get());
        break;
    }

    LS::Win32::ClearRT(m_context.Get(), m_renderTarget.Get(), clearColor);
}

void RenderD3D11::Draw() noexcept
{
    LS::Win32::Present1(m_swapchain.GetSwapChain1().Get(), 0);
    ++m_frameIndex;
}

void RenderD3D11::Resize(uint32_t width, uint32_t height) noexcept
{
    namespace WRL = Microsoft::WRL;
    if (m_renderTarget)
        m_renderTarget = nullptr;
    if (m_dsv)
        m_dsv = nullptr;
    // TODO: Might need to clear device context state (immediate) before doing this?
    m_swapchain.Resize(width, height);

    if (!InitializeRtvAndDsv())
    {
        LS_LOG_ERROR("Failed to re-initialize the RTV or DSV during resize");
    }
}

void LS::Win32::RenderD3D11::SetFps(uint32_t fps) noexcept
{
    m_settings.FPSTarget = fps;
}

auto LS::Win32::RenderD3D11::Initialize() noexcept -> LS::System::ErrorCode
{
    const LS::System::ErrorCode ec = m_device.InitDevice(m_settings, m_window);
    if (!ec)
    {
        return ec;
    }
    m_context = m_device.GetImmediateContext();

    const LS::System::ErrorCode ecSwap = InitializeSwapchain();
    if (!ecSwap)
    {
        return ecSwap;
    }
    // Create Render Target View
    if (!InitializeRtvAndDsv())
    {
        return LS::System::CreateFailCode("Failed to create Render Target View or Depth Stencil View");
    }

    return LS::System::CreateSuccessCode();
}

void RenderD3D11::SetPipeline(const PipelineStateDX11* state) noexcept
{
    assert(state && "state cannot be null");
    if (!state)
        return;

    auto context = m_device.GetImmediateContext();

    const auto findStrides = [&state]() -> std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, std::vector<ID3D11Buffer*>>
        {
            std::vector<uint32_t> offsets;
            std::vector<uint32_t> strides;
            std::vector<ID3D11Buffer*> buffers;
            for (const auto& b : state->VertexBuffers)
            {
                offsets.emplace_back(0);
                strides.emplace_back(b.BufferDesc.StructureByteStride);
                buffers.emplace_back(b.Buffer.Get());
            }

            return { offsets, strides, buffers };
        };

    // Input Assembler Stage Setup //
    const auto [s, o, b] = findStrides();
    context->IASetVertexBuffers(0, static_cast<UINT>(state->VertexBuffers.size()), b.data(), s.data(), o.data());

    if (state->IndexBuffer.Buffer)
    {
        context->IASetIndexBuffer(state->IndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    }

    context->IASetInputLayout(state->InputLayout.Get());
    context->IASetPrimitiveTopology(state->PrimitiveTopology);

    // Load Shaders //
    assert(state->VertexShader && "A vertex shader cannot be null");
    context->VSSetShader(state->VertexShader.Get(), nullptr, 0);
    assert(state->PixelShader && "A pixel shader cannot be null");
    context->PSSetShader(state->PixelShader.Get(), nullptr, 0);

    if (state->GeometryShader)
    {
        context->GSSetShader(state->GeometryShader.Get(), nullptr, 0);
    }

    if (state->HullShader)
    {
        context->HSSetShader(state->HullShader.Get(), nullptr, 0);
    }

    if (state->DomainShader)
    {
        context->DSSetShader(state->DomainShader.Get(), nullptr, 0);
    }

    if (state->ComputeShader)
    {
        context->CSSetShader(state->ComputeShader.Get(), nullptr, 0);
    }

    // Load Shader Buffers //

    // Load Output Merger States //
    context->OMSetBlendState(state->BlendState.State.Get(), state->BlendState.BlendFactor, state->BlendState.SampleMask);

    const auto findRTs = [&state]() -> std::vector<ID3D11RenderTargetView*>
        {
            std::vector<ID3D11RenderTargetView*> views;
            for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            {
                if (!state->RTStage.RTViews[i])
                    break;
                views.emplace_back(state->RTStage.RTViews[i].Get());
            }

            return views;
        };
    const auto rts = findRTs();
    
    if (!state->DSStage.State)
    {
        context->OMSetDepthStencilState(nullptr, state->DSStage.StencilRef);
    }
    else
    {
        context->OMSetDepthStencilState(state->DSStage.State.Get(), state->DSStage.StencilRef);
    }
}

void RenderD3D11::Shutdown() noexcept
{
}

void RenderD3D11::AttachToWindow(LS::LSWindowBase* window) noexcept
{

}

auto LS::Win32::RenderD3D11::GetDevice() const noexcept -> ID3D11Device*
{
    return m_device.GetDevice().Get();
}

auto LS::Win32::RenderD3D11::GetDeviceCom() const noexcept -> WRL::ComPtr<ID3D11Device>
{
    return m_device.GetDevice();
}

auto LS::Win32::RenderD3D11::GetSwapChainCom() const noexcept -> WRL::ComPtr<IDXGISwapChain1>
{
    return m_swapchain.GetSwapChain1();
}

auto LS::Win32::RenderD3D11::GetDeviceContextCom() const noexcept -> WRL::ComPtr<ID3D11DeviceContext>
{
    return m_device.GetImmediateContext();
}

auto LS::Win32::RenderD3D11::CreateVertexShader(std::span<std::byte> data) const  noexcept -> WRL::ComPtr<ID3D11VertexShader>
{
    if (data.size() == 0)
        return nullptr;

    WRL::ComPtr<ID3D11VertexShader> shader;
    if (!IsCompiled(data))
    {
        LS_LOG_ERROR("The given shader code was not compiled.\n");
        return nullptr;
    }

    // It's compiled already;
    auto result = CreateVertexShaderFromByteCode(m_device.GetDevice().Get(), data, &shader);
    if (FAILED(result))
    {
        LS_LOG_ERROR(LS::Win32::HresultToDx11Error(result));
        return nullptr;
    }
    return shader;
}

auto LS::Win32::RenderD3D11::CreatePixelShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11PixelShader>
{
    if (data.size() == 0)
        return nullptr;
    // If compiled with fxc it will contain DXBC at the front of the compiled code
    WRL::ComPtr<ID3D11PixelShader> shader;

    if (!IsCompiled(data))
    {
        LS_LOG_ERROR("The given shader code was not compiled.\n");
        return nullptr;
    }

    // It's compiled already;
    auto result = CreatePixelShaderFromByteCode(m_device.GetDevice().Get(), data, &shader);
    if (FAILED(result))
    {
        LS_LOG_ERROR(LS::Win32::HresultToDx11Error(result));
        return nullptr;
    }
    return shader;
}

auto LS::Win32::RenderD3D11::CreateGeometryShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11GeometryShader>
{
    if (data.size() == 0)
        return nullptr;
    // If compiled with fxc it will contain DXBC at the front of the compiled code
    WRL::ComPtr<ID3D11GeometryShader> shader;

    if (!IsCompiled(data))
    {
        LS_LOG_ERROR("The given shader code was not compiled.\n");
        return nullptr;
    }

    // It's compiled already;
    auto result = CreateGeometryShaderFromByteCode(m_device.GetDevice().Get(), data, &shader);
    if (FAILED(result))
    {
        LS_LOG_ERROR(LS::Win32::HresultToDx11Error(result));
        return nullptr;
    }
    return shader;
}

auto LS::Win32::RenderD3D11::CreateDomainShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11DomainShader>
{
    if (data.size() == 0)
        return nullptr;
    // If compiled with fxc it will contain DXBC at the front of the compiled code
    WRL::ComPtr<ID3D11DomainShader> shader;

    if (!IsCompiled(data))
    {
        LS_LOG_ERROR("The given shader code was not compiled.\n");
        return nullptr;
    }

    // It's compiled already;
    auto result = CreateDomainShaderFromByteCode(m_device.GetDevice().Get(), data, &shader);
    if (FAILED(result))
    {
        LS_LOG_ERROR(LS::Win32::HresultToDx11Error(result));
        return nullptr;
    }
    return shader;
}

auto LS::Win32::RenderD3D11::CreateHullShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11HullShader>
{
    if (data.size() == 0)
        return nullptr;
    // If compiled with fxc it will contain DXBC at the front of the compiled code
    WRL::ComPtr<ID3D11HullShader> shader;

    if (!IsCompiled(data))
    {
        LS_LOG_ERROR("The given shader code was not compiled.\n");
        return nullptr;
    }

    // It's compiled already;
    auto result = CreateHullShaderFromByteCode(m_device.GetDevice().Get(), data, &shader);
    if (FAILED(result))
    {
        LS_LOG_ERROR(LS::Win32::HresultToDx11Error(result));
        return nullptr;
    }
    return shader;
}

auto LS::Win32::RenderD3D11::CreateComputeShader(std::span<std::byte> data) const noexcept -> WRL::ComPtr<ID3D11ComputeShader>
{
    if (data.size() == 0)
        return nullptr;
    // If compiled with fxc it will contain DXBC at the front of the compiled code
    WRL::ComPtr<ID3D11ComputeShader> shader;

    if (!IsCompiled(data))
    {
        LS_LOG_ERROR("The given shader code was not compiled.\n");
        return nullptr;
    }

    // It's compiled already;
    auto result = CreateComputeShaderFromByteCode(m_device.GetDevice().Get(), data, &shader);
    if (FAILED(result))
    {
        LS_LOG_ERROR(LS::Win32::HresultToDx11Error(result));
        return nullptr;
    }
    return shader;
}

auto LS::Win32::RenderD3D11::CreateImmediateCommand() const noexcept -> RenderCommandD3D11
{
    return RenderCommandD3D11(m_device.GetDevice().Get(), COMMAND_MODE::IMMEDIATE);
}

auto LS::Win32::RenderD3D11::CreateDeferredCommand() const noexcept -> RenderCommandD3D11
{
    return RenderCommandD3D11(m_device.GetDevice().Get(), COMMAND_MODE::DEFERRED);
}

auto LS::Win32::RenderD3D11::InitializeSwapchain() noexcept -> LS::System::ErrorCode
{
    namespace WRL = Microsoft::WRL;
    Nullable<WRL::ComPtr<IDXGIDevice1>> dev = m_device.GetIDXGDevice1();
    if (!dev)
    {
        return LS::System::CreateFailCode("Failed to obtain IDXGIDevice1 interface from DeviceD3D11"); // TODO: Report error or throw?
    }

    auto dxgiDev = dev.value();

    WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    HRESULT hr = dxgiDev->GetAdapter(&dxgiAdapter);
    if (FAILED(hr))
    {
        return LS::System::CreateFailCode("Failed to obtain IDXGI Adapter interface");
    }

    WRL::ComPtr<IDXGIFactory2> factory;
    hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
    {
        return LS::System::CreateFailCode("Failed to create IDXGIFactory2");
    }
    const auto swDesc1 = BuildSwapchainDesc1(m_settings.FrameBufferCount, m_settings.Width, 
        m_settings.Height, m_settings.PixelFormat);

    bool status = m_swapchain.InitializeForHwnd(factory.Get(), static_cast<HWND>(m_window->GetHandleToWindow()),
        m_device.GetDevice().Get(), &swDesc1);

    if (!status)
    {
        return LS::System::CreateFailCode("Failed to initialize the swapchain with HWND");
    }

    return LS::System::CreateSuccessCode();
}

auto LS::Win32::RenderD3D11::BuildInputLayout(std::span<std::byte> compiledByteCode) -> Nullable<WRL::ComPtr<ID3D11InputLayout>>
{
    const auto elements = BuildFromReflection(compiledByteCode);

    if (!elements)
    {
        return std::nullopt;
    }

    const auto& e = elements.value();
    WRL::ComPtr<ID3D11InputLayout> inputLayout;
    const auto result = m_device.CreateInputLayout(e.data(), static_cast<UINT>(e.size()), compiledByteCode, &inputLayout);
    if (FAILED(result))
    {
        return std::nullopt;
    }

    return inputLayout;
}

void LS::Win32::RenderD3D11::ExecuteRenderCommand(const LS::Win32::RenderCommandD3D11& command) noexcept
{
    if (command.GetMode() == COMMAND_MODE::IMMEDIATE)
        return;

    const auto pCommList = command.GetCommandList();
    const auto devCon = m_device.GetImmediateContext();

    if (!pCommList)
    {
        return;
    }

    devCon->ExecuteCommandList(pCommList.value(), false);
}

bool LS::Win32::RenderD3D11::InitializeRtvAndDsv() noexcept
{
    // Create RTV // 
    Nullable<WRL::ComPtr<ID3D11RenderTargetView>> rtv = LS::Win32::CreateRenderTargetViewFromSwapChain(m_device.GetDevice(), m_swapchain.GetSwapChain());
    if (!rtv)
    {
        LS_LOG_ERROR("Failed to create render target view during Resize");
        return false;
    }
    m_renderTarget = rtv.value();

    // Create DSV //
    //DXGI_FORMAT_D32_FLOAT or DXGI_FORMAT_D24_UNORM_S8_UINT
    Nullable<WRL::ComPtr<ID3D11DepthStencilView>> dsvResult = LS::Win32::CreateDepthStencilViewFromSwapChain(m_device.GetDevice(),
        m_swapchain.GetSwapChain(), m_dsBuffer, DXGI_FORMAT_D24_UNORM_S8_UINT);

    if (!dsvResult)
    {
        LS_LOG_ERROR("Failed to create Depth Stencil View during Resize");
        return false;
    }

    m_dsv = dsvResult.value();
    LS::Utils::SetDebugName(m_dsv.Get(), "Renderer Depth Stencil View");
    LS::Utils::SetDebugName(m_dsBuffer.Get(), "Renderer Depth Buffer");
    return true;
}
