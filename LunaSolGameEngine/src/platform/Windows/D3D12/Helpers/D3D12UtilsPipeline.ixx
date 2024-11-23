module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>
#include <wrl/client.h>
export module D3D12Lib.D3D12Utils.Pipeline;
import <span>;

// This module is intended for helping in build up the necessary components to construct a Pipeline State Object

namespace LS::Platform::Dx12
{
    export class RootParamBuilder
    {
    private:
        std::vector<D3D12_ROOT_PARAMETER1> m_rootParams;

    public:
        RootParamBuilder(uint32_t paramCount) : m_rootParams(paramCount)
        {
        }

        auto CreateCbvParam(uint32_t shaderRegister, uint32_t registerSpace,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const;

        auto CreateSrvParam(uint32_t shaderRegister, uint32_t registerSpace,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const;

        auto CreateUavParam(uint32_t shaderRegister, uint32_t registerSpace,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const;

        auto CreateDescTableParam(std::span<D3D12_DESCRIPTOR_RANGE1> descRanges,
            D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const;

        auto CreateConstantsParam(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const;

        auto GetParams() -> std::vector<D3D12_ROOT_PARAMETER1>
        {
            return m_rootParams;
        }
    };
}

export namespace LS::Platform::Dx12
{
    auto CreateRootParamBuilder(uint32_t paramCount) -> RootParamBuilder
    {
        return RootParamBuilder(paramCount);
    }

    auto CreateRootSignature(ID3D12Device* pDevice, std::span<D3D12_ROOT_PARAMETER1> rootParams, 
        std::span<D3D12_STATIC_SAMPLER_DESC> samplers, D3D12_ROOT_SIGNATURE_FLAGS flags) -> Microsoft::WRL::ComPtr<ID3D12RootSignature>;
    auto CreateDescriptorRange1(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t numDescriptors, uint32_t baseShaderRegister, uint32_t registerSpace = 0, uint32_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) -> D3D12_DESCRIPTOR_RANGE;
    auto CreateDescriptorRange1(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t numDescriptors, uint32_t baseShaderRegister, uint32_t registerSpace = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE, uint32_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) -> D3D12_DESCRIPTOR_RANGE1;
    
}

namespace LS::Platform::Dx12
{
    auto CreateRootParamAsCBV(uint32_t shaderRegister, uint32_t registerSpace,
        D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER;

    auto CreateRootParamAsSRV(uint32_t shaderRegister, uint32_t registerSpace,
        D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER;

    auto CreateRootParamAsUAV(uint32_t shaderRegister, uint32_t registerSpace,
        D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER;
    
    auto CreateRootParamAsDescTable(std::span<D3D12_DESCRIPTOR_RANGE> descRanges,
        D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER;
    
    auto CreateRootParamAsConstants(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace,
        D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER;

    // ROOT PARAM 1 TYPES //
    auto CreateRoot1ParamAsCBV(uint32_t shaderRegister, uint32_t registerSpace,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1;
    
    auto CreateRoot1ParamAsSRV(uint32_t shaderRegister, uint32_t registerSpace,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1;

    auto CreateRoot1ParamAsUAV(uint32_t shaderRegister, uint32_t registerSpace,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1;

    auto CreateRoot1ParamAsDescTable(std::span<D3D12_DESCRIPTOR_RANGE1> descRanges,
        D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1;

    auto CreateRoot1ParamAsConstants(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1;
}

module : private;

using namespace LS::Platform::Dx12;

auto CreateRootSignature(ID3D12Device* pDevice, std::span<D3D12_ROOT_PARAMETER1> rootParams, 
    std::span<D3D12_STATIC_SAMPLER_DESC> samplers, D3D12_ROOT_SIGNATURE_FLAGS flags) -> Microsoft::WRL::ComPtr<ID3D12RootSignature>
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

    rootSignatureDesc.Init_1_1(static_cast<UINT>(rootParams.size()), rootParams.data(), 
        static_cast<UINT>(samplers.size()), samplers.size() == 0 ? nullptr : samplers.data(), 
        flags);

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

    return rootSignature;
}

auto CreateDescriptorRange1(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t numDescriptors, uint32_t baseShaderRegister, 
    uint32_t registerSpace, uint32_t offset) -> D3D12_DESCRIPTOR_RANGE
{
    return CD3DX12_DESCRIPTOR_RANGE(type, numDescriptors, baseShaderRegister, registerSpace, offset);
}

auto CreateDescriptorRange1(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t numDescriptors, uint32_t baseShaderRegister, 
    uint32_t registerSpace, D3D12_DESCRIPTOR_RANGE_FLAGS flags, uint32_t offset) -> D3D12_DESCRIPTOR_RANGE1
{
    return CD3DX12_DESCRIPTOR_RANGE1(type, numDescriptors, baseShaderRegister, registerSpace, flags, offset);
}

auto CreateRootParamAsCBV(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER
{
    CD3DX12_ROOT_PARAMETER rp;
    rp.InitAsConstantBufferView(shaderRegister, registerSpace, shaderVis);
    return rp;
}

auto CreateRootParamAsSRV(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER
{
    CD3DX12_ROOT_PARAMETER rp;
    rp.InitAsShaderResourceView(shaderRegister, registerSpace, shaderVis);
    return rp;
}

auto CreateRootParamAsUAV(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER
{
    CD3DX12_ROOT_PARAMETER rp;
    rp.InitAsUnorderedAccessView(shaderRegister, registerSpace, shaderVis);
    return rp;
}

auto CreateRootParamAsDescTable(std::span<D3D12_DESCRIPTOR_RANGE> descRanges,
    D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER
{
    CD3DX12_ROOT_PARAMETER rp;
    rp.InitAsDescriptorTable(static_cast<UINT>(descRanges.size()), descRanges.data(), shaderVis);
    return rp;
}

auto CreateRootParamAsConstants(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER
{
    CD3DX12_ROOT_PARAMETER rp;
    rp.InitAsConstants(num32BitValues, shaderRegister, registerSpace, shaderVis);
    return rp;
}

// ROOT PARAM 1 TYPES //
auto CreateRoot1ParamAsCBV(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1
{
    CD3DX12_ROOT_PARAMETER1 rp;
    rp.InitAsConstantBufferView(shaderRegister, registerSpace, flags, shaderVis);
    return rp;

}

auto CreateRoot1ParamAsSRV(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1
{
    CD3DX12_ROOT_PARAMETER1 rp;
    rp.InitAsShaderResourceView(shaderRegister, registerSpace, flags, shaderVis);
    return rp;

}

auto CreateRoot1ParamAsUAV(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1
{
    CD3DX12_ROOT_PARAMETER1 rp;
    rp.InitAsShaderResourceView(shaderRegister, registerSpace, flags, shaderVis);
    return rp;
}

auto CreateRoot1ParamAsDescTable(std::span<D3D12_DESCRIPTOR_RANGE1> descRanges,
    D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1
{
    CD3DX12_ROOT_PARAMETER1 rp;
    rp.InitAsDescriptorTable(static_cast<UINT>(descRanges.size()), descRanges.data(), shaderVis);
    return rp;
}

auto CreateRoot1ParamAsConstants(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace, D3D12_SHADER_VISIBILITY shaderVis) -> D3D12_ROOT_PARAMETER1
{
    CD3DX12_ROOT_PARAMETER1 rp;
    rp.InitAsConstants(num32BitValues, shaderRegister, registerSpace, shaderVis);
    return rp;
}

auto RootParamBuilder::CreateCbvParam(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const
{
    const auto rp = ::CreateRoot1ParamAsCBV(shaderRegister, registerSpace, flags, shaderVis);
    m_rootParams.push_back(rp);
    return this;
}

auto RootParamBuilder::CreateSrvParam(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const
{
    const auto rp = ::CreateRoot1ParamAsCBV(shaderRegister, registerSpace, flags, shaderVis);
    m_rootParams.push_back(rp);
    return this;
}

auto RootParamBuilder::CreateUavParam(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const
{
    const auto rp = ::CreateRoot1ParamAsCBV(shaderRegister, registerSpace, flags, shaderVis);
    m_rootParams.push_back(rp);
    return this;
}

auto RootParamBuilder::CreateDescTableParam(std::span<D3D12_DESCRIPTOR_RANGE1> descRanges,
    D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const
{
    const auto rp = ::CreateRoot1ParamAsDescTable(descRanges, shaderVis);
    m_rootParams.push_back(rp);
    return this;
}

auto RootParamBuilder::CreateConstantsParam(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder * const
{
    const auto rp = ::CreateRoot1ParamAsConstants(num32BitValues, shaderRegister, registerSpace, shaderVis);
    m_rootParams.push_back(rp);
    return this;
}