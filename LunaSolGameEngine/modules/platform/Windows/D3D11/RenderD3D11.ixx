module;
#include <cstdint>
#include <array>
#include <span>
#include <wrl/client.h>
#include <d3d11_4.h>
#include "engine/EngineDefines.h"
export module D3D11.RenderD3D11;

import Engine.LSWindow;
import Engine.LSDevice;
import Engine.EngineCodes;
import D3D11.PipelineFactory;
import D3D11.Device;
import DirectXCommon;

namespace LS
{
    export class IRenderable
    {
    public:
        IRenderable() noexcept;
        virtual ~IRenderable() noexcept;

        virtual void Draw() noexcept = 0;
        virtual void Update(LS::DX::DXCamera* camera) noexcept = 0;
    };

    export struct TimeStep
    {
        uint64_t timeMs;
        uint64_t deltaMs;
    };
}

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    class RenderD3D11
    {
    public:
        RenderD3D11(LS::LSDeviceSettings settings);
        ~RenderD3D11();
        
        auto Initialize() noexcept -> LS::System::ErrorCode;
        void Clear(std::array<float, 4> clearColor) noexcept;
        void SetPipeline(PipelineStateDX11* pipelineState) noexcept;
        void Update(LS::DX::DXCamera* camera, const TimeStep& timeStep) noexcept;
        void RenderObjects(std::span<LS::IRenderable*> objs) noexcept;
        void Draw() noexcept;
        void Shutdown() noexcept;
        void Resize(uint32_t width, uint32_t height) noexcept;
        void AttachToWindow(LS::LSWindowBase* window) noexcept;

    protected:
        LS::LSWindowBase*       m_window;
        DeviceD3D11             m_device;
        LS::LSDeviceSettings    m_settings;

        WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;
    };
}

module : private;

using namespace LS::Win32;
namespace WRL = Microsoft::WRL;

RenderD3D11::RenderD3D11(LS::LSDeviceSettings settings) : m_settings(settings),
m_window(settings.Window)
{
}

RenderD3D11::~RenderD3D11()
{
    Shutdown();
}

auto LS::Win32::RenderD3D11::Initialize() noexcept -> LS::System::ErrorCode
{
    LS::System::ErrorCode ec = m_device.InitDevice(m_settings);
    if (!ec)
    {
        return ec;
    }

    m_device.CreateRTVFromBackBuffer(&m_renderTarget);
}

void RenderD3D11::Clear(std::array<float, 4> clearColor) noexcept
{

}

void RenderD3D11::SetPipeline(PipelineStateDX11* pipelineState) noexcept
{
}

void RenderD3D11::Update(LS::DX::DXCamera* cameram, const TimeStep& timeStep) noexcept
{
}

void RenderD3D11::RenderObjects(std::span<LS::IRenderable*> objs) noexcept
{
}

void RenderD3D11::Draw() noexcept
{
}

void RenderD3D11::Shutdown() noexcept
{
}

void RenderD3D11::Resize(uint32_t width, uint32_t height) noexcept
{
}

void RenderD3D11::AttachToWindow(LS::LSWindowBase* window) noexcept
{
}
