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

        void CreateRasterizerState(const RasterizerInfo& rastInfo, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept;
        void CreateDepthStencil(const DepthStencil& depthStencil, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept;
        void CreateBlendState(const LSBlendState& blendState, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept;
        void CompileShaders(const ShaderMap& shaders, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept;
        void CreateShaderInput(const LSShaderInputSignature& inputSignature, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept;
        void CreateRenderTarget(const LSTextureInfo& renderTarget, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept;
    };
}

module :private;
namespace LS
{
    /*void PipelineFactory::CreateRasterizerState(const RasterizerInfo& rastInfo, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept
    {
    }

    void PipelineFactory::CreateDepthStencil(const DepthStencil& depthStencil, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept
    {
    }

    void PipelineFactory::CreateBlendState(const LSBlendState& blendState, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept
    {

    }

    void PipelineFactory::CompileShaders(const ShaderMap& shaders, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept
    {

    }

    void PipelineFactory::CreateShaderInput(const LSShaderInputSignature& inputSignature, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept
    {

    }

    void PipelineFactory::CreateRenderTarget(const LSTextureInfo& renderTarget, Ref<LSPipelineState>& pPipeline, Ref<LSDevice>& pDevice) noexcept
    {

    }*/

}