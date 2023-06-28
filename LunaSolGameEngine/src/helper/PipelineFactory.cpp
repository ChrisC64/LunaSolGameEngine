#include <optional>
#include <memory>

import Helper.PipelineFactory;

auto LS::BuildPipelienFactory([[maybe_unused]] Ref<ILSDevice>& pDevice) noexcept -> Ref<IPipelineFactory>
{
    return {};
}