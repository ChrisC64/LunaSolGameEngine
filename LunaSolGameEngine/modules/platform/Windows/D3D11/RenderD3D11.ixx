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
import D3D11.Utils;
import D3D11.PipelineFactory;
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
    class RenderD3D11
    {
    public:
        RenderD3D11(LS::LSDeviceSettings settings);
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

    protected:
        LS::LSWindowBase*       m_window;
        DeviceD3D11             m_device;
        LS::LSDeviceSettings    m_settings;

        WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;
        WRL::ComPtr<ID3D11DeviceContext> m_context;
    };
}