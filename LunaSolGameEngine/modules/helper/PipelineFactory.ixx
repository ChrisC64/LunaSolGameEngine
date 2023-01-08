export module Helper.PipelineFactory;

import Engine.LSDevice;
import Engine.LSDefinitions;
import Data.LSShader;

export namespace LS
{
    class PipelineFactory
    {
    public:
        //TODO: Build an interface Device/Context as they will be needed to create the pipeline state
        PipelineFactory() noexcept = default;
        ~PipelineFactory() = default;

        void CreateRasterizerState(const RasterizerInfo& rastInfo, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept;
        void CreateDepthStencil(const DepthStencil& depthStencil, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept;
        void CreateBlendState(const LSBlendState& blendState, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept;
        void CompileShaders(const ShaderMap& shaders, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept;
        void CreateShaderInput(const LSShaderInputSignature& inputSignature, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept;
        void CreateRenderTarget(const LSTextureInfo& renderTarget, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept;
    };
}

module :private;
namespace LS
{
    /*void PipelineFactory::CreateRasterizerState(const RasterizerInfo& rastInfo, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept
    {
    }

    void PipelineFactory::CreateDepthStencil(const DepthStencil& depthStencil, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept
    {
    }

    void PipelineFactory::CreateBlendState(const LSBlendState& blendState, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept
    {

    }

    void PipelineFactory::CompileShaders(const ShaderMap& shaders, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept
    {

    }

    void PipelineFactory::CreateShaderInput(const LSShaderInputSignature& inputSignature, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept
    {

    }

    void PipelineFactory::CreateRenderTarget(const LSTextureInfo& renderTarget, Ref<PipelineDescriptor>& pPipeline, Ref<ILSDevice>& pDevice) noexcept
    {

    }*/

}