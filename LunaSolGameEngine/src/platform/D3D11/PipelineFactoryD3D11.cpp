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

    return false;
}

PipelineD3D11 LS::Win32::D3D11PipelineFactory::CreatePipelineD3D11(const PipelineDescriptor& pipeline) noexcept
{
    return PipelineD3D11();
}
