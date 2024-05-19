module;
#include <string_view>
export module Helper.PipelineFactory;

import Engine.LSDevice;
import Engine.EngineCodes;
import Engine.Defines;

export namespace LS
{
    class IPipelineFactory
    {
    protected:
        IPipelineFactory() noexcept = default;

    public:

        //TODO: Build an interface Device/Context as they will be needed to create the pipeline state
        virtual ~IPipelineFactory() = default;

        [[nodiscard]] virtual auto CreatePipelineState(const PipelineDescriptor& pipeline, std::string_view guid) noexcept -> LS::System::ErrorCode = 0;
    };

    auto BuildPipelienFactory(Ref<ILSDevice>& pDevice) noexcept -> Ref<IPipelineFactory>;
}