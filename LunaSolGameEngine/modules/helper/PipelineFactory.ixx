export module Helper.PipelineFactory;

import Engine.LSDevice;
import Engine.LSDefinitions;
import Data.LSShader;

export namespace LS
{
    class IPipelineFactory
    {
    protected:
        IPipelineFactory() noexcept = default;

    public:

        //TODO: Build an interface Device/Context as they will be needed to create the pipeline state
        virtual ~IPipelineFactory() = default;

        [[nodiscard]]
        virtual bool CreatePipelineState(const PipelineDescriptor& pipeline) noexcept = 0;
    };
}