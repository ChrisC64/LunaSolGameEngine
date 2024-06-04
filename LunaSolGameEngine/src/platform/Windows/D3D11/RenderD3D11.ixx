module;
//#include <cstdint>
//#include <array>
//#include <span>
//#include <filesystem>
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
    
    class RenderCommandD3D11;

    class RenderD3D11
    {
    public:
        RenderD3D11(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow);
        ~RenderD3D11();

        // Interface Design (WIP)
        void Clear(const std::array<float, 4>& clearColor) noexcept;
        void Draw() noexcept;

        // Class Members // 
        auto Initialize() noexcept -> LS::System::ErrorCode;
        void SetPipeline(const PipelineStateDX11* state) noexcept;
        void LoadVertexBuffer(uint32_t startSlot, const PipelineStateDX11* state) noexcept;
        void LoadIndexBuffer(uint32_t offset, const PipelineStateDX11* state) noexcept;
        void Update(const LS::DX::DXCamera& camera) noexcept;
        void RenderObjects() noexcept;
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
    const std::string check = { (char)(data[0]), (char)data[1], (char)data[2], (char)data[3] };
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

void RenderD3D11::Clear(const std::array<float, 4>& clearColor) noexcept
{
    LS::Win32::ClearRT(m_context.Get(), m_renderTarget.Get(), clearColor);
}

void RenderD3D11::Draw() noexcept
{
    LS::Win32::Present1(m_device.GetSwapChain().Get(), 1);
}

auto LS::Win32::RenderD3D11::Initialize() noexcept -> LS::System::ErrorCode
{
    LS::System::ErrorCode ec = m_device.InitDevice(m_settings, m_window);
    if (!ec)
    {
        return ec;
    }

    auto hresult = m_device.CreateRTVFromBackBuffer(&m_renderTarget);
    if (FAILED(hresult))
    {
        return LS::System::CreateFailCode(LS::Win32::HresultToDx11Error(hresult));
    }
    m_context = m_device.GetImmediateContext();

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
    // FIX-ME: Fix this issue as I removed View, it should be bound by the user, so let's split up some
    // rendering tasks instead of having one whole "operation" like this. 
    /*if (!state->DSStage.View)
    {
        if (rts.size() == 0)
        {
            context->OMSetRenderTargets(0, nullptr, nullptr);
        }
        else
        {
            context->OMSetRenderTargets(static_cast<UINT>(rts.size()), rts.data(), nullptr);
        }
    }
    else
    {
        if (rts.size() == 0)
        {
            context->OMSetRenderTargets(0, nullptr, state->DSStage.View.Get());
        }
        else
        {
            context->OMSetRenderTargets(static_cast<UINT>(rts.size()), rts.data(), state->DSStage.View.Get());
        }
    }*/

    if (!state->DSStage.State)
    {
        context->OMSetDepthStencilState(nullptr, state->DSStage.StencilRef);
    }
    else
    {
        context->OMSetDepthStencilState(state->DSStage.State.Get(), state->DSStage.StencilRef);
    }
}

void RenderD3D11::LoadVertexBuffer(uint32_t startSlot, const PipelineStateDX11* state) noexcept
{
}

void RenderD3D11::LoadIndexBuffer(uint32_t offset, const PipelineStateDX11* state) noexcept
{
}

void RenderD3D11::Update(const LS::DX::DXCamera& camera) noexcept
{
}

void RenderD3D11::RenderObjects() noexcept
{
}

void RenderD3D11::Shutdown() noexcept
{
}

void RenderD3D11::Resize(uint32_t width, uint32_t height) noexcept
{
    if (m_renderTarget)
        m_renderTarget = nullptr;
    m_device.ResizeSwapchain(width, height);
    const HRESULT result = m_device.CreateRTVFromBackBuffer(&m_renderTarget);
    if (FAILED(result))
    {
        LS_LOG_ERROR(HresultToDx11Error(result));
    }
}

void RenderD3D11::AttachToWindow(LS::LSWindowBase* window) noexcept
{

}

auto LS::Win32::RenderD3D11::GetDevice() noexcept -> ID3D11Device*
{
    return m_device.GetDevice().Get();
}

auto LS::Win32::RenderD3D11::GetDeviceCom() noexcept -> WRL::ComPtr<ID3D11Device>
{
    return m_device.GetDevice();
}

auto LS::Win32::RenderD3D11::GetSwapChainCom() noexcept -> WRL::ComPtr<IDXGISwapChain1>
{
    return m_device.GetSwapChain();
}

auto LS::Win32::RenderD3D11::GetDeviceContextCom() noexcept -> WRL::ComPtr<ID3D11DeviceContext>
{
    return m_device.GetImmediateContext();
}

auto LS::Win32::RenderD3D11::CreateVertexShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11VertexShader>
{
    if (data.size() == 0)
        return nullptr;

    const std::string check = { (char)(data[0]), (char)data[1], (char)data[2], (char)data[3] };

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

auto LS::Win32::RenderD3D11::CreatePixelShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11PixelShader>
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

auto LS::Win32::RenderD3D11::CreateGeometryShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11GeometryShader>
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

auto LS::Win32::RenderD3D11::CreateDomainShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11DomainShader>
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

auto LS::Win32::RenderD3D11::CreateHullShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11HullShader>
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

auto LS::Win32::RenderD3D11::CreateComputeShader(std::span<std::byte> data) noexcept -> WRL::ComPtr<ID3D11ComputeShader>
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

auto LS::Win32::RenderD3D11::CreateImmediateCommand() noexcept -> RenderCommandD3D11
{
    return RenderCommandD3D11(m_device.GetDevice().Get(), COMMAND_MODE::IMMEDIATE);
}

auto LS::Win32::RenderD3D11::CreateDeferredCommand() noexcept -> RenderCommandD3D11
{
    return RenderCommandD3D11(m_device.GetDevice().Get(), COMMAND_MODE::DEFERRED);
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

auto LS::Win32::RenderD3D11::ExecuteRenderCommand(const LS::Win32::RenderCommandD3D11& command) noexcept
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