export module Helper.PipelineFactory;

import Engine.LSDevice;
import Engine.LSDefinitions;
import Data.LSShader;

export namespace LS
{
    class PipelineFactory
    {
    private:
        class Impl;
        Ref<Impl> m_pimpl;

    public:
        //TODO: Build an interface Device/Context as they will be needed to create the pipeline state
        PipelineFactory();
        ~PipelineFactory() = default;

        LSOptional<LSPipelineState> BuildPipelineState(const LSDrawState& drawState,
            const LSBlendState& blendState, const DepthStencil& depthStencil,
            const ShaderMap Shaders, const LSShaderInputSignature& shaderInput,
            PRIMITIVE_TOPOLOGY topology, std::span<SamplerMap> samplers = {},
            std::span<TextureMap> textures = {}, std::span<BufferMap> buffers = {});
    };
}