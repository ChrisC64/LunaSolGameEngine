export module Helper.PipelineFactory;

import Engine.LSDevice;
import Data.LSDataTypes;
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
        virtual auto CreatePipelineState(const PipelineDescriptor& pipeline) noexcept -> Nullable <GuidUL> = 0;
    };

    auto BuildPipelienFactory(Ref<ILSDevice>& pDevice) noexcept -> Ref<IPipelineFactory>;
}