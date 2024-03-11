module;
#include <cstdint>
#include <array>
#include <span>
#include <filesystem>
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
    enum class RENDER_MODE
    {
        IMMEDIATE,
        DEFERRED
    };

    class RenderCommandD3D11
    {
    public:
        RenderCommandD3D11(ID3D11Device* pDevice, RENDER_MODE mode);
        ~RenderCommandD3D11();

        // Bind Shaders to Pipeline //
        BindVS();
        BindPS();
        BindGS();
        BindCS();
        BindHS();
        BindDS();

        // Bind Commands for Resources for Shaders //
        SetTexture();
        SetConstantBuffer();
        SetVertexBuffer();
        SetIndexBuffer();
        SetSampler();

        // Set Input For this Draw State //
        SetRenderTarget();
        SetRasterizerState();
        SetPrimTopology();
        SetInputLayout;
        SetViewPort();
        SetDepthStencilState();
        SetBlendState();

        // Draw Commands //
        Clear();
        ClearDepth();
        DrawIndexed();
        DrawVerts();

        // State Operations //
        Finish(); // @brief Finishes recording command list if DEFERRED, else nothing for IMMEDIATE
        ClearState(); // @brief Resets to default state
        FlushCommands();// @brief Expunge all commands recorded up to this point
    private:
        WRL::ComPtr<ID3D11DeviceContext4> m_context;
    };

    class RenderD3D11
    {
    public:
        RenderD3D11(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow);
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

        /**
         * @brief Creates an input layout from the given compiled bytecode. This will not work if it was not compiled first.
         * 
         * @param compiledByteCode The compiled byte code of the shader
         * @return Nullable<WRL::ComPtr<ID3D11InputLayout>> A nullable object of WRL::ComPtr<ID3D11InputLayout>> 
         */
        auto BuildInputLayout(std::span<std::byte> compiledByteCode) -> Nullable<WRL::ComPtr<ID3D11InputLayout>>;

    protected:
        LS::LSWindowBase*       m_window;
        DeviceD3D11             m_device;
        LS::LSDeviceSettings    m_settings;

        WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;
        WRL::ComPtr<ID3D11DeviceContext> m_context;
    };
}