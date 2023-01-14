import D3D11.PipelineFactory;

#include "LSEFramework.h"

import Engine.Common;
import D3D11Lib;
import LSData;

using namespace LS;
using namespace LS::Win32;



D3D11PipelineFactory::D3D11PipelineFactory(SharedRef<DeviceD3D11>& deviceD3D11) : m_pDevice(deviceD3D11)
{
}

bool D3D11PipelineFactory::CreatePipelineState(const PipelineDescriptor& pipeline) noexcept
{
    assert(m_pDevice);
    if (!m_pDevice)
        return false;

    auto pipelineD3D = CreatePipelineD3D11(pipeline);

    m_pipelines.emplace_back(pipelineD3D);
    return true;
}

PipelineD3D11 LS::Win32::D3D11PipelineFactory::CreatePipelineD3D11(const PipelineDescriptor& pipeline) noexcept
{
    PipelineD3D11 out;
    ID3D11Device5* device = m_pDevice->GetDevice().Get();
    auto rasterizer = CreateRasterizerState2(device, pipeline.RasterizeState);
    if (rasterizer)
    {
        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> rast = rasterizer.value();
        out.RasterizerState = rast;
    }

    auto blendState = CreateBlendState1(device, pipeline.BlendState);
    if (blendState)
    {
        Microsoft::WRL::ComPtr<ID3D11BlendState1> blend = blendState.value();
        out.BlendState = blend;
    }

    return out;
}
