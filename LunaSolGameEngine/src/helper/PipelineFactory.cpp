#include "LSEFramework.h"

import Helper.PipelineFactory;

auto LS::BuildPipelienFactory([[maybe_unused]] Ref<ILSDevice>& pDevice) noexcept -> Ref<IPipelineFactory>
{
    return {};
}