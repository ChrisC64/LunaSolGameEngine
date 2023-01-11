module;
#include "LSEFramework.h"

export module D3D11.PipelineFactory;
import Helper.PipelineFactory;
import LSData;
import Engine.Common;
import Util;
import Engine.LSDefinitions;
import D3D11.Device;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    using ShaderMapD3D11 = std::unordered_map<SHADER_TYPE, std::vector<std::byte>>;
    
    struct BufferD3D11
    {
        D3D11_BIND_FLAG         BindFlag;
        uint16_t                BindSLot;
        WRL::ComPtr<ID3D11Buffer> Buffer;
    };

    struct TextureD3D11
    {
        uint16_t                    BindSlot;
        //TODO: Should I try Variant with 1D/2D/3D texture instead for a little more...saftey?
        WRL::ComPtr<ID3D11Resource> Texture;
    };

    struct SamplerD3D11
    {
        uint16_t                        BindSlot;
        WRL::ComPtr<ID3D11SamplerState> Sampler;
    };

    struct PipelineD3D11
    {
        D3D_PRIMITIVE_TOPOLOGY                  PrimitiveTopology;
        WRL::ComPtr<ID3D11RasterizerState>      RasterizerState;
        WRL::ComPtr<ID3D11BlendState>           BlendState;
        WRL::ComPtr<ID3D11DepthStencilState>    DepthStencilState;
        WRL::ComPtr<ID3D11DepthStencilView>     DepthStencilView;
        ShaderMapD3D11                          Shaders;
        WRL::ComPtr<ID3D11InputLayout>          InputLayout;
        WRL::ComPtr<ID3D11Resource>             RenderTarget;
        WRL::ComPtr<ID3D11RenderTargetView>     RenderTargetView;

        std::vector<BufferD3D11>                VertexBuffers;
        std::vector<BufferD3D11>                IndexBuffers;
        std::vector<SamplerD3D11>               Samplers;
        std::vector<TextureD3D11>               Textures;
    };

    class D3D11PipelineFactory : IPipelineFactory
    {
    public:
        D3D11PipelineFactory(SharedRef<DeviceD3D11>& deviceD3D11);
        ~D3D11PipelineFactory() = default;

        [[nodiscard]]
        virtual bool CreatePipelineState(const PipelineDescriptor& pipeline) noexcept final;

        PipelineD3D11 CreatePipelineD3D11(const PipelineDescriptor& pipeline) noexcept;

    private:
        SharedRef<DeviceD3D11> m_pDevice;
    };
}