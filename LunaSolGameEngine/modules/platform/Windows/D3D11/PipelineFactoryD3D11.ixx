module;
#include <vector>
#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <d3d11_4.h>
#include <wrl/client.h>
#include "engine/EngineDefines.h"

export module D3D11.PipelineFactory;
import Helper.PipelineFactory;
import Engine.App;
import Engine.EngineCodes; 
import Util;
import D3D11.Device;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    using ShaderMapD3D11 = std::unordered_map<SHADER_TYPE, std::vector<std::byte>>;
    
    struct BufferD3D11
    {
        D3D11_BUFFER_DESC           BufferDesc;
        WRL::ComPtr<ID3D11Buffer>   Buffer;
    };

    template<class T>
    struct VertexBufferD3D11 : BufferD3D11
    {
        Ref<T> Obj;
    };

    struct IndexBufferD3D11 : BufferD3D11
    {
        std::vector<uint32_t> IndexBuffer;
    };

    template<class T>
    struct ConstantBufferD3D11 : BufferD3D11
    {
        Ref<T> Obj;
    };

    struct TextureD3D11
    {
        uint16_t                    BindSlot;
        WRL::ComPtr<ID3D11Resource> Texture;
    };

    struct SamplerD3D11
    {
        uint16_t                        BindSlot;
        WRL::ComPtr<ID3D11SamplerState> Sampler;
    };

    struct BlendStage
    {
        WRL::ComPtr<ID3D11BlendState> State;
        float                         BlendFactor[4]{ 1.0f, 1.0f, 1.0f, 1.0f};
        uint32_t                      SampleMask = 0xFFFFFFFF;
    };

    struct DepthStencilStage
    {
        WRL::ComPtr<ID3D11DepthStencilView>     View;
        WRL::ComPtr<ID3D11DepthStencilState>    State;
        uint32_t                                StencilRef;
    };

    struct PipelineStateDX11
    {
        D3D_PRIMITIVE_TOPOLOGY                  PrimitiveTopology;
        WRL::ComPtr<ID3D11RasterizerState>      RasterizerState;
        DepthStencilStage                       DSStage;
        WRL::ComPtr<ID3D11VertexShader>         VertexShader;
        WRL::ComPtr<ID3D11PixelShader>          PixelShader;
        WRL::ComPtr<ID3D11GeometryShader>       GeometryShader;
        WRL::ComPtr<ID3D11HullShader>           HullShader;
        WRL::ComPtr<ID3D11DomainShader>         DomainShader;
        WRL::ComPtr<ID3D11InputLayout>          InputLayout;
        WRL::ComPtr<ID3D11Resource>             RenderTarget;
        WRL::ComPtr<ID3D11RenderTargetView>     RTViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];

        BlendStage                              BlendState;
        std::vector<BufferD3D11>                VertexBuffers;
        BufferD3D11                             IndexBuffer;
        std::vector<SamplerD3D11>               Samplers;
        std::vector<TextureD3D11>               Textures;
    };

    class D3D11PipelineFactory final : public IPipelineFactory
    {
    public:
        D3D11PipelineFactory() = default;
        ~D3D11PipelineFactory() = default;

        D3D11PipelineFactory& operator=(D3D11PipelineFactory&&) = default;
        D3D11PipelineFactory(D3D11PipelineFactory&&) = default;
        D3D11PipelineFactory& operator=(const D3D11PipelineFactory&) = delete;
        D3D11PipelineFactory(const D3D11PipelineFactory&) = delete;

        void Init(SharedRef<DeviceD3D11>& device) noexcept;
        [[nodiscard]] auto CreatePipelineState(const PipelineDescriptor& pipeline, std::string_view) noexcept -> LS::System::ErrorCode final;

        PipelineStateDX11 CreatePipelineD3D11(const PipelineDescriptor& pipeline);

    private:
        SharedRef<DeviceD3D11> m_pDevice;
        std::vector<PipelineStateDX11> m_pipelines;
    };
}