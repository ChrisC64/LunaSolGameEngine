module;
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
export module D3D12Lib.D3D12Utils.Pipeline;
import <span>;
import <filesystem>;
import <unordered_map>;

import Helper.IO;
import Engine.Shader;
import DirectXCommon.D3DCompiler;

export namespace LS
{
    enum class DESCRIPTOR_RANGE_FLAGS
    {
        NONE = 0,
        DATA_VOLATILE = 0x2,
        DATA_STATIC_WHILE_SET_AT_EXECUTE = 0x4,
        DATA_STATIC = 0x8
    };

    enum SHADER_VISIBILITY
    {
        VISIBILITY_ALL = 0,
        VISIBILITY_VERTEX = 1,
        VISIBILITY_HULL = 2,
        VISIBILITY_DOMAIN = 3,
        VISIBILITY_GEOMETRY = 4,
        VISIBILITY_PIXEL = 5,
        VISIBILITY_AMPLIFICATION = 6,
        VISIBILITY_MESH = 7
    };

    enum class DESCRIPTOR_RANGE_TYPE
    {
        SRV = 0,
        UAV = (SRV + 1),
        CBV = (UAV + 1),
        SAMPLER = (CBV + 1)
    };
}

// This module is intended for helping in build up the necessary components to construct a PSO
export namespace LS::Platform::Dx12
{
    class RootParamBuilder
    {
    private:
        std::vector<D3D12_ROOT_PARAMETER1> m_rootParams;
        std::vector<D3D12_DESCRIPTOR_RANGE1> m_ranges;

    public:
        explicit RootParamBuilder(uint32_t paramCount) : m_rootParams(),
            m_ranges{}
        {
            m_rootParams.reserve(paramCount);
        }

        auto CreateCbvParam(uint32_t shaderRegister, uint32_t registerSpace,
            DESCRIPTOR_RANGE_FLAGS flags, SHADER_VISIBILITY shaderVis) -> RootParamBuilder&;

        auto BeginDescriptorRange(uint32_t count) -> RootParamBuilder&;

        auto CreateDescRange(DESCRIPTOR_RANGE_TYPE rangeType,
            UINT numDescriptors, UINT baseShaderRegister, 
            UINT registerSpace = 0, DESCRIPTOR_RANGE_FLAGS flags = DESCRIPTOR_RANGE_FLAGS::NONE,
            UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) -> RootParamBuilder&;
        
        auto EndDescriptorRange(uint32_t rootParamIndex, LS::SHADER_VISIBILITY visibility);

        auto GetParams() -> std::vector<D3D12_ROOT_PARAMETER1>
        {
            return m_rootParams;
        }

        auto GetDescriptorRanges() -> std::vector<D3D12_DESCRIPTOR_RANGE1>
        {
            return m_ranges;
        }

    private:

        auto CreateCbvParam(uint32_t shaderRegister, uint32_t registerSpace,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&;

        auto CreateSrvParam(uint32_t shaderRegister, uint32_t registerSpace,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&;

        auto CreateUavParam(uint32_t shaderRegister, uint32_t registerSpace,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&;

        auto CreateDescTableParam(std::span<D3D12_DESCRIPTOR_RANGE1> descRanges,
            D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&;

        auto CreateConstantsParam(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&;
    };

    class InputLayoutBuilder
    {
    private:
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

    public:
        explicit InputLayoutBuilder(uint32_t count) : m_inputLayout()
        {
            m_inputLayout.reserve(count);
        }

        void AddElement(LPCSTR semanticName, UINT semanticIndex, DXGI_FORMAT format, UINT inputSlot, UINT alignedByteOffset, D3D12_INPUT_CLASSIFICATION inputSlotClass, UINT instanceDataStepRate);

        auto GetLayout() -> std::vector<D3D12_INPUT_ELEMENT_DESC>
        {
            return m_inputLayout;
        }
        
        auto GetLayout() const -> const std::vector<D3D12_INPUT_ELEMENT_DESC>&
        {
            return m_inputLayout;
        }
    };

    class Dx12PsoBuilder
    {
    private:
        RootParamBuilder m_rpBuilder;
        InputLayoutBuilder m_ilBuilder;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
        std::unordered_map<LS::SHADER_TYPE, std::vector<std::byte>> m_byteCodeMap;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    public:
        Dx12PsoBuilder(uint32_t rootParamCount, uint32_t inputLayoutCount) : m_rpBuilder(rootParamCount),
            m_ilBuilder(inputLayoutCount),
            m_psoDesc{}
        {
            
            // Generic Defaults for most startup state
            m_psoDesc.DepthStencilState.DepthEnable = false;
            m_psoDesc.DepthStencilState.StencilEnable = false;
            m_psoDesc.SampleMask = UINT_MAX;
            m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            m_psoDesc.NumRenderTargets = 1;
            m_psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            // No MSAA
            m_psoDesc.SampleDesc.Count = 1;
            m_psoDesc.SampleDesc.Quality = 0;

            // Default States //
            //TODO: Check this, might need to alter this to see test demo
            m_psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            m_psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        }

        auto GetRootParamBuilder() -> RootParamBuilder&
        {
            return m_rpBuilder;
        }

        auto GetInputLayoutBuilder() -> InputLayoutBuilder&
        {
            return m_ilBuilder;
        }

        auto SetBlendState(const D3D12_BLEND_DESC& blendState) -> void
        {
            m_psoDesc.BlendState = blendState;
        }

        auto SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& dss) -> void
        {
            m_psoDesc.DepthStencilState = dss;
        }

        auto EnableDepth(D3D12_DEPTH_WRITE_MASK writeMask, D3D12_COMPARISON_FUNC func) -> void
        {
            m_psoDesc.DepthStencilState.DepthEnable = true;
            m_psoDesc.DepthStencilState.DepthWriteMask = writeMask;
            m_psoDesc.DepthStencilState.DepthFunc = func;
        }

        auto DisableDepth() -> void
        {
            m_psoDesc.DepthStencilState.DepthEnable = false;
        }

        auto EnableStencil(UINT8 readMask, UINT8 writeMask, D3D12_DEPTH_STENCILOP_DESC frontFace, D3D12_DEPTH_STENCILOP_DESC backFace) -> void
        {
            m_psoDesc.DepthStencilState.StencilEnable = true;
            m_psoDesc.DepthStencilState.StencilReadMask = readMask;
            m_psoDesc.DepthStencilState.StencilWriteMask = writeMask;
            m_psoDesc.DepthStencilState.FrontFace = frontFace;
            m_psoDesc.DepthStencilState.BackFace = backFace;
        }

        auto DisableStencil() -> void
        {
            m_psoDesc.DepthStencilState.StencilEnable = false;
        }

        auto SetDepthStencilViewFormat(DXGI_FORMAT format) -> void
        {
            m_psoDesc.DSVFormat = format;
        }

        /**
         * @brief Load the supplied bytecode of a shader
         * @param byteCode Compiled shader bytecode
         * @param type Shader type
         * @return 
         */
        auto LoadShader(const std::vector<std::byte>& byteCode, LS::SHADER_TYPE type) -> void
        {
            if (byteCode.size() == 0)
                return;

            CD3DX12_SHADER_BYTECODE shader(byteCode.data(), byteCode.size());
            using enum LS::SHADER_TYPE;
            switch (type)
            {
            case VERTEX:
                m_psoDesc.VS = shader;
                break;
            case PIXEL:
                m_psoDesc.PS = shader;
                break;
            case GEOMETRY:
                m_psoDesc.GS = shader;
                break;
            case COMPUTE:
                //m_psoDesc.CS = shader;
                break;
            case HULL:
                m_psoDesc.HS = shader;
                break;
            case DOM:
                m_psoDesc.DS = shader;
                break;
            default:
                break;
            }

            m_byteCodeMap[type] = byteCode;
        }

        /**
         * @brief Loads a compiled shader file 
         * @param compiledFile The pathway to the compiled byte code
         * @param type Type this shader belongs to in the pipeline
         * @return 
         */
        auto LoadShader(std::filesystem::path compiledFile, LS::SHADER_TYPE type) -> void
        {
            const auto result = LS::DX::DxcLoadFile(compiledFile);
            if (!result)
            {
#ifdef _DEBUG
                throw std::runtime_error(std::format("Failed to read file {}", compiledFile.string()).c_str());
#endif
                return;
            }

            const auto compiled = result.value();
            LoadShader(compiled, type);
        }

        auto CompileShaderDxc(std::filesystem::path filepath, LS::SHADER_TYPE type, const wchar_t* entryPoint, const wchar_t* target)
        {
            const auto result = LS::DX::DxcCompileFile(filepath, entryPoint, target);
            if (!result)
            {
#ifdef _DEBUG
                throw std::runtime_error(std::format("Failed to compile file: {}", filepath.string()).c_str());
#endif
                return;
            }

            const auto compiled = result.value();
            LoadShader(compiled, type);
        }

        /**
         * @brief Compile the shader based on the supplied shader options
         * @param options Options for the compiler
         * @return 
         */
        auto CompileShaderFromFileFxc(const LS::CompileShaderOpts& options) -> void
        {
            const auto result = LS::DX::FxcCompileShader(options);
            if (result.HasError())
            {
#ifdef _DEBUG
                throw std::runtime_error(result.ErrorMsg.c_str());
#endif
                return;
            }
            
            CD3DX12_SHADER_BYTECODE shader(result.ByteCode.data(), result.ByteCode.size());
            using enum LS::SHADER_TYPE;
            switch (options.ShaderType)
            {
            case VERTEX:
                m_psoDesc.VS = shader;
                break;
            case PIXEL:
                m_psoDesc.PS = shader;
                break;
            case GEOMETRY:
                m_psoDesc.GS = shader;
                break;
            case COMPUTE:
                //m_psoDesc.CS = shader;
                break;
            case HULL:
                m_psoDesc.HS = shader;
                break;
            case DOM:
                m_psoDesc.DS = shader;
                break;
            default:
                break;
            }
        }

        auto SetRasterizerState(D3D12_RASTERIZER_DESC rasterizer) -> void
        {
            m_psoDesc.RasterizerState = rasterizer;
        }

        auto SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) -> void
        {
            m_psoDesc.PrimitiveTopologyType = topology;
        }

        auto SetRenderTarget(D3D12_RT_FORMAT_ARRAY rtArray) -> void
        {
            m_psoDesc.NumRenderTargets = rtArray.NumRenderTargets;
            std::copy(std::begin(rtArray.RTFormats), std::end(rtArray.RTFormats), std::begin(m_psoDesc.RTVFormats));
        }

        auto BuildPSO(ID3D12Device* pDevice) -> Microsoft::WRL::ComPtr<ID3D12PipelineState>;

        auto GetRootSignature() -> Microsoft::WRL::ComPtr<ID3D12RootSignature>
        {
            return m_rootSignature;
        }
    };
}// end of exported Namespace LS::PlatformDx12//


namespace LS::Platform::Dx12
{
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

        rootSignatureDesc.Init_1_1(
            static_cast<UINT>(rootParams.size()), rootParams.size() == 0 ? nullptr : rootParams.data(),
            static_cast<UINT>(samplers.size()), samplers.size() == 0 ? nullptr : samplers.data(),
            flags
        );

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
        if (FAILED(hr))
        {
            throw std::runtime_error("D3DX12SerializeVersionedRootSignature failed");
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr))
        {
            throw std::runtime_error("CreateRootSignature failed");
        }
        return rootSignature;
    }

    auto CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t numDescriptors, uint32_t baseShaderRegister,
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
}

module : private;

using namespace LS::Platform::Dx12;

auto RootParamBuilder::CreateCbvParam(uint32_t shaderRegister, uint32_t registerSpace, DESCRIPTOR_RANGE_FLAGS flags, SHADER_VISIBILITY shaderVis) -> RootParamBuilder&
{
    return CreateCbvParam(shaderRegister, registerSpace, (D3D12_ROOT_DESCRIPTOR_FLAGS)flags, (D3D12_SHADER_VISIBILITY)shaderVis);
}

auto LS::Platform::Dx12::RootParamBuilder::BeginDescriptorRange(uint32_t count) -> RootParamBuilder&
{
    m_ranges.reserve(count);
    return *this;
}

auto RootParamBuilder::CreateDescRange(DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace, DESCRIPTOR_RANGE_FLAGS flags, UINT offsetInDescriptorsFromTableStart)  -> RootParamBuilder&
{
    auto descRange = CreateDescriptorRange1((D3D12_DESCRIPTOR_RANGE_TYPE)rangeType, numDescriptors, baseShaderRegister, registerSpace, (D3D12_DESCRIPTOR_RANGE_FLAGS)flags, offsetInDescriptorsFromTableStart);
    m_ranges.push_back(descRange);
    return *this;
}

auto LS::Platform::Dx12::RootParamBuilder::EndDescriptorRange(uint32_t rootParamIndex, LS::SHADER_VISIBILITY visibility)
{
    if (rootParamIndex > m_rootParams.size() - 1)
    {
        OutputDebugString(std::format(L"Root Param Index: {} is out of bounds from given root Params: {}", rootParamIndex, m_rootParams.size()).c_str());
        return;
    }
    
    CD3DX12_ROOT_PARAMETER1 rp;
    rp.InitAsDescriptorTable(m_ranges.size(), m_ranges.data(), (D3D12_SHADER_VISIBILITY)visibility);
    m_rootParams.insert(m_rootParams.begin() + rootParamIndex, rp);
}

// Root Param Builder // 
auto RootParamBuilder::CreateCbvParam(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&
{
    const auto rp = ::CreateRoot1ParamAsCBV(shaderRegister, registerSpace, flags, shaderVis);
    m_rootParams.push_back(rp);
    return *this;
}

auto RootParamBuilder::CreateSrvParam(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&
{
    const auto rp = ::CreateRoot1ParamAsCBV(shaderRegister, registerSpace, flags, shaderVis);
    m_rootParams.push_back(rp);
    return *this;
}

auto RootParamBuilder::CreateUavParam(uint32_t shaderRegister, uint32_t registerSpace,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&
{
    const auto rp = ::CreateRoot1ParamAsCBV(shaderRegister, registerSpace, flags, shaderVis);
    m_rootParams.push_back(rp);
    return *this;
}

auto RootParamBuilder::CreateDescTableParam(std::span<D3D12_DESCRIPTOR_RANGE1> descRanges,
    D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&
{
    const auto rp = ::CreateRoot1ParamAsDescTable(descRanges, shaderVis);
    m_rootParams.push_back(rp);
    return *this;
}

auto RootParamBuilder::CreateConstantsParam(uint32_t num32BitValues, uint32_t shaderRegister, uint32_t registerSpace, D3D12_SHADER_VISIBILITY shaderVis) -> RootParamBuilder&
{
    const auto rp = ::CreateRoot1ParamAsConstants(num32BitValues, shaderRegister, registerSpace, shaderVis);
    m_rootParams.push_back(rp);
    return *this;
}

// Input Layout Builder //
void InputLayoutBuilder::AddElement(LPCSTR semanticName, UINT semanticIndex, DXGI_FORMAT format, UINT inputSlot, UINT alignedByteOffset, D3D12_INPUT_CLASSIFICATION inputSlotClass, UINT instanceDataStepRate)
{
    D3D12_INPUT_ELEMENT_DESC desc{ 
        .SemanticName = semanticName, .SemanticIndex = semanticIndex, 
        .Format = format, .InputSlot = inputSlot, .AlignedByteOffset = alignedByteOffset, 
        .InputSlotClass = inputSlotClass, .InstanceDataStepRate = instanceDataStepRate
    };
    m_inputLayout.emplace_back(desc);
}

auto Dx12PsoBuilder::BuildPSO(ID3D12Device* pDevice) -> Microsoft::WRL::ComPtr<ID3D12PipelineState>
{
    if (!pDevice)
        return nullptr;

    auto rootParams = m_rpBuilder.GetParams();

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    m_rootSignature = CreateRootSignature(pDevice, 
        rootParams, 
        std::span<D3D12_STATIC_SAMPLER_DESC>{}, 
        rootSignatureFlags);

    m_psoDesc.pRootSignature = m_rootSignature.Get();
    const auto& il = m_ilBuilder.GetLayout();

    m_psoDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ 
        .pInputElementDescs = il.size() == 0 ? nullptr : il.data(),
        .NumElements = static_cast<UINT>(il.size())
    };

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
    pDevice->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&pso));
    if (!pso)
    {
        return nullptr;
    }
    
    return pso;
}