module;
#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <span>
#include <algorithm>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <directx/d3dx12.h>
#include <thread>
#include <optional>
#include <stdexcept>
#include <mutex>
#include <ranges>
#include <semaphore>
#include <functional>
#include <DirectXMath.h>
#include <d3dx12.h>
#include <format>
export module CubeApp;

import LSData;
import Engine.App;
import D3D12Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Helper.PipelineFactory;
import Util.MSUtils;
import DXGIHelper;

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

template<class T, size_t Count>
struct Vector
{
    std::array<T, Count> Vec;
};

export struct Vertex
{
    Vector<float, 3> position;
    Vector<float, 4> color;
};

export struct VertexPT
{
    Vector<float, 4> position;
    Vector<float, 2> uv;
};

namespace WRL = Microsoft::WRL;
using namespace DirectX;
namespace gt::dx12
{
    struct VertexPositionColor
    {
        XMFLOAT3 Position;
        XMFLOAT3 Color;
    };

    static VertexPositionColor g_vertices[8] = {
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
    };

    static WORD g_indices[36] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } g_pipelineStateStream;

    template <class T, size_t Size>
    using ComPtrArray = std::array<WRL::ComPtr<T>, Size>;
    constexpr auto NUM_CONTEXT = 3;
    constexpr auto FRAME_COUNT = NUM_CONTEXT;

    struct FrameContext
    {
        // Manages a heap for the command lists. This cannot be reset while the CommandList is still in flight on the GPU
        WRL::ComPtr<ID3D12CommandAllocator>     CommandAllocator;
        // Sends commands to the GPU - represents this frames commands
        WRL::ComPtr<ID3D12GraphicsCommandList>  CommandList;
        // Singal value between the GPU and CPU to perform synchronization. 
        UINT64                                  FenceValue;
    };

    export class DX12CubeApp : public LS::LSApp
    {
    public:
        DX12CubeApp(uint32_t width, uint32_t height, std::wstring_view title) : m_frameBuffer(FRAME_COUNT)
        {
            Window = LS::BuildWindow(width, height, title);
            m_settings.Width = width;
            m_settings.Height = height;
            m_settings.FeatureLevel = D3D_FEATURE_LEVEL_12_0;
            m_settings.Hwnd = (HWND)Window->GetHandleToWindow();
            m_device = std::make_unique<LS::Platform::Dx12::DeviceD3D12>(m_settings);
        }
        ~DX12CubeApp() = default;

        auto Initialize(int argCount = 0, char* argsV[] = nullptr) -> LS::System::ErrorCode override;
        void Run() override;

        //[[nodiscard]] bool LoadPipeline();
        //void LoadAssets();
        //void PopulateCommandList();
        //void OnRender();
        void OnDestroy();
        //void OnUpdate();
        //void WaitForPreviousFrame();
        void CreateCommandQueue();

    private:
        // Tutorial Stuff //
        WRL::ComPtr<IDXGIFactory4> m_pFactory;
        // My stuff to replace above // 
        Ref<LS::Platform::Dx12::DeviceD3D12> m_device;
        LS::Platform::Dx12::CommandQueueDx12 m_directQueue{ D3D12_COMMAND_LIST_TYPE_DIRECT };
        LS::Platform::Dx12::CommandQueueDx12 m_copyQueue{ D3D12_COMMAND_LIST_TYPE_COPY };
        Ref<LS::Platform::Dx12::CommandListDx12> m_commandList;
        LS::Platform::Dx12::D3D12Settings m_settings{};
        LS::Platform::Dx12::FrameBufferDxgi m_frameBuffer;
        LS::Platform::Dx12::DescriptorHeapDx12 m_heapRtv{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_CONTEXT };
        LS::Platform::Dx12::DescriptorHeapDx12 m_heapSrv{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE };
        LS::Platform::Dx12::DescriptorHeapDx12 m_heapDsv{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 };

        // pipeline objects
        WRL::ComPtr<ID3D12Resource>								m_texture = nullptr;
        WRL::ComPtr<ID3D12Resource> m_cubeVb;
        D3D12_VERTEX_BUFFER_VIEW m_cubeVbView;

        WRL::ComPtr<ID3D12Resource> m_cubeIb;
        D3D12_INDEX_BUFFER_VIEW m_cubeIbView;

        WRL::ComPtr<ID3D12Resource> m_depthBuffer;
        WRL::ComPtr<IDXGIFactory7> m_factory;
        WRL::ComPtr<ID3D12RootSignature> m_cubeRootSignature;
        WRL::ComPtr<ID3D12PipelineState> m_cubePipelineState;
        UINT													m_rtvDescriptorSize = 0;
        HANDLE													m_scWaitableHandle = nullptr;
        CD3DX12_VIEWPORT										m_viewport;
        CD3DX12_RECT											m_scissorRect;
        float m_fov;
        XMMATRIX m_cubeModelMat;
        XMMATRIX m_cubeViewMat;
        XMMATRIX m_cubeProjMat;
        bool m_contentLoaded;

        bool CreateDevice();

        void CreateSwapchain();
        void CreateDescriptors();
        void UpdateBufferResource(WRL::ComPtr<ID3D12GraphicsCommandList>& commandLIst,
            ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
            size_t numElements, size_t elementSize, const void* bufferData,
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
        bool LoadContent();
        void ResizeDepthBuffer(int width, int height);
        void Render(float r, float g, float b, float a);
        void DemoRun();
        void SetupState();
        void Update();
    };

    float													g_aspectRatio;

    HANDLE													g_drawFinished;
    HANDLE													g_prepWorkDone;
    HANDLE													g_beginRender;

    inline auto CreateDefaultBuffer(WRL::ComPtr<ID3D12Device> device, WRL::ComPtr<ID3D12GraphicsCommandList>& cmdList, const void* initData, uint64_t byteSize, WRL::ComPtr<ID3D12Resource>& uploadBuffer, D3D12_RESOURCE_STATES finalState, std::optional<std::wstring_view> defaultName = std::nullopt, std::optional<std::wstring_view> uploadName = std::nullopt) -> WRL::ComPtr<ID3D12Resource>;
    inline auto GenerateTextureData(uint32_t textureWidth, uint32_t textureHeight, uint32_t pixelSize) -> std::vector<UINT8>;
    void CreateTileSampleTexture(WRL::ComPtr<ID3D12Device>& device, WRL::ComPtr<ID3D12Resource>& texture, uint32_t textureWidth, uint32_t textureHeight, uint32_t pixelSize, WRL::ComPtr<ID3D12Resource>& textureUploadHeap, WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, WRL::ComPtr<ID3D12DescriptorHeap>& srvHeap);
}

module : private;

using namespace gt;

inline auto gt::dx12::CreateDefaultBuffer(WRL::ComPtr<ID3D12Device> device, WRL::ComPtr<ID3D12GraphicsCommandList>& cmdList,
    const void* initData, uint64_t byteSize, WRL::ComPtr<ID3D12Resource>& uploadBuffer, D3D12_RESOURCE_STATES finalState,
    std::optional<std::wstring_view> defaultName, std::optional<std::wstring_view> uploadName) -> WRL::ComPtr<ID3D12Resource>
{
    WRL::ComPtr<ID3D12Resource> defaultBuffer;

    // Creat Default Buffer - this resides on the GPU's dedicated memory (fast)
    auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapDefault,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())
    ));

    // Upload type is needed to get the data onto the GPU - this is the GPU's shared memory, which is slow
    // but grants the GPU/CPU to share this memory to write and read from (slow)
    auto heapUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapUpload,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())
    ));

    // Create subresource data to copy into the default barrier
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = initData;
    subresourceData.RowPitch = byteSize;
    subresourceData.SlicePitch = subresourceData.RowPitch;

    auto defaultBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), 
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &defaultBarrier);

    UpdateSubresources<1>(cmdList.Get(), defaultBuffer.Get(), uploadBuffer.Get(), 0, 
        0, 1, &subresourceData);

    auto defaultBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), 
        D3D12_RESOURCE_STATE_COPY_DEST, finalState);
    cmdList->ResourceBarrier(1, &defaultBarrier2);

    if (defaultName)
    {
        defaultBuffer->SetName(defaultName.value().data());
    }

    if (uploadName)
    {
        uploadBuffer->SetName(uploadName.value().data());
    }

    return defaultBuffer;
}

inline auto gt::dx12::GenerateTextureData(uint32_t textureWidth, 
    uint32_t textureHeight, uint32_t pixelSize) -> std::vector<UINT8>
{
    const UINT rowPitch = textureWidth * pixelSize;
    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = textureWidth >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * textureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += pixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}

void gt::dx12::CreateTileSampleTexture(WRL::ComPtr<ID3D12Device>& device, 
    WRL::ComPtr<ID3D12Resource>& texture, uint32_t textureWidth, 
    uint32_t textureHeight, uint32_t pixelSize, 
    WRL::ComPtr<ID3D12Resource>& textureUploadHeap, 
    WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, 
    WRL::ComPtr<ID3D12DescriptorHeap>& srvHeap)
{
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = textureWidth;
    textureDesc.Height = textureHeight;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapDefault,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
    auto buffer = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    // Create the GPU upload buffer.
    ThrowIfFailed(device->CreateCommittedResource(
        &heapUpload,
        D3D12_HEAP_FLAG_NONE,
        &buffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap)));

    // Copy data to the intermediate upload heap and then schedule a copy 
    // from the upload heap to the Texture2D.
    std::vector<UINT8> textureData = GenerateTextureData(textureWidth, textureHeight, pixelSize);

    D3D12_SUBRESOURCE_DATA textureSubresourceData = {};
    textureSubresourceData.pData = &textureData[0];
    textureSubresourceData.RowPitch = textureWidth * pixelSize;
    textureSubresourceData.SlicePitch = textureSubresourceData.RowPitch * textureHeight;

    UpdateSubresources(commandList.Get(), texture.Get(), textureUploadHeap.Get(), 
        0, 0, 1, &textureSubresourceData);
    auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), 
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &transition);

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(texture.Get(), &srvDesc, srvHeap->GetCPUDescriptorHandleForHeapStart());
    texture->SetName(L"texture");
}

bool gt::dx12::DX12CubeApp::CreateDevice()
{
    // [DEBUG] Enable debug interface
    UINT flags = 0;
#ifdef _DEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    auto factory = LS::Win32::CreateFactory(flags).value();
    auto hr = factory.As(&m_pFactory);
    LS::Utils::ThrowIfFailed(hr, "Failed to obtain IDXGIFactory4 interface");
    // Find the best graphics card (best performing one, with single GPU systems, this should be the default)
    auto adapterOptional = LS::Win32::GetHardwareAdapter(m_pFactory.Get(), true);
    if (!adapterOptional)
    {
        //std::cout << "Failed to obtain adapter with hardware requirement.\n";
        return false;
    }

    WRL::ComPtr<IDXGIAdapter1> adapter = adapterOptional.value();

    if (!m_device->CreateDevice(adapter))
    {
        //std::cout << "Failed to create the DX12 Device class.\n";
        return false;
    }
    return true;
}

//bool gt::dx12::DX12CubeApp::LoadAssets()
//{
//    // Create an empty root signature for shader.hlsl
//    {
//        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
//        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
//
//        WRL::ComPtr<ID3DBlob> signature;
//        WRL::ComPtr<ID3DBlob> error;
//        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
//        ThrowIfFailed(m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature)));
//    }
//
//    // Create a root signature for our texture_effect.hlsl
//    // Create the root signature.
//    {
//        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
//
//        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
//        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
//
//        if (FAILED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
//        {
//            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
//        }
//
//        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
//        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
//
//        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
//        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
//
//        D3D12_STATIC_SAMPLER_DESC sampler = {};
//        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
//        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//        sampler.MipLODBias = 0;
//        sampler.MaxAnisotropy = 0;
//        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
//        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
//        sampler.MinLOD = 0.0f;
//        sampler.MaxLOD = D3D12_FLOAT32_MAX;
//        sampler.ShaderRegister = 0;
//        sampler.RegisterSpace = 0;
//        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
//
//        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
//        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
//
//        WRL::ComPtr<ID3DBlob> signature;
//        WRL::ComPtr<ID3DBlob> error;
//        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
//        ThrowIfFailed(m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature2)));
//    }
//    // Create pipeline states and associate to command allocators since we have an array of them
//    auto count = 0;
//    for (auto& fc : m_frameContexts)
//    {
//        ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&fc.CommandAllocator)));
//        ThrowIfFailed(m_pDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&fc.CommandList)));
//        fc.CommandAllocator->SetName(std::format(L"FC Command Allocator {}", count).c_str());
//        fc.CommandList->SetName(std::format(L"FC Command List {}", count).c_str());
//        ++count;
//    }
//
//    //g_pCurrFrameContext = &m_frameContexts[m_currFrameIndex];
//    // Create the pipeline state, which includes compiling and loading shaders.
//    {
//        WRL::ComPtr<ID3DBlob> vertexShader;
//        WRL::ComPtr<ID3DBlob> pixelShader;
//        WRL::ComPtr<ID3DBlob> vertexShader2;
//        WRL::ComPtr<ID3DBlob> pixelShader2;
//
//#if defined(_DEBUG)
//        // Enable better shader debugging with the graphics debugging tools.
//        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//        UINT compileFlags = 0;
//#endif
//
//        ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
//        ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
//        // Define the vertex input layout.
//        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
//        {
//            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
//        };
//
//        //TODO: Create another PSO that uses the root signature and input element descriptions for our second textured triangle
//        // then see if we can display both on the scene at once. 
//        // Describe and create the graphics pipeline state object (PSO).
//        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
//        psoDesc.pRootSignature = m_pRootSignature.Get();
//        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
//        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
//        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//        psoDesc.DepthStencilState.DepthEnable = FALSE;
//        psoDesc.DepthStencilState.StencilEnable = FALSE;
//        psoDesc.SampleMask = UINT_MAX;
//        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//        psoDesc.NumRenderTargets = 1;
//        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//        psoDesc.SampleDesc.Count = 1;
//        ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState)));
//        // Bundle Test // 
//        {
//            ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_pBundleAllocator)));
//            ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pBundleAllocator.Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&m_pBundleList)));
//        }
//
//        // Create root signature for texture_effect.hlsl
//        ThrowIfFailed(D3DCompileFromFile(L"texture_effect.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader2, nullptr));
//        ThrowIfFailed(D3DCompileFromFile(L"texture_effect.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader2, nullptr));
//        D3D12_INPUT_ELEMENT_DESC inputElementDescs2[] =
//        {
//            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
//        };
//
//        psoDesc.InputLayout = { inputElementDescs2, _countof(inputElementDescs2) };
//        psoDesc.pRootSignature = m_pRootSignature2.Get();
//        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader2.Get());
//        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader2.Get());
//        ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineStatePT)));
//    }
//
//    {
//        // A fence is used for synchronization
//        if (m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)) != S_OK)
//            return false;
//
//        // Update the fence value, from startup, this should be 0, and thus the next frame we'll be creating will be the first frame (back buffer, as 0 is currently in front)
//        //m_frameContexts[m_frameResourceIndex].FenceValue++;
//        //g_pCurrFrameContext->FenceValue++;
//
//        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, L"Fence Event");
//        if (m_fenceEvent == nullptr)
//        {
//            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
//        }
//    }
//
//    return true;
//}

//void gt::dx12::DX12CubeApp::LoadVertexDataToGpu()
//{
//    // For simplicity, I've opted to make two command lists with two separate local command allocators. This is so I can then put these two command lists to work
//    // right away, without waiting on one or the other. These jobs only happen once, so it's only done this way for quick and easy, over "correctness" of loading data onto the GPU.
//    WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
//    ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_pCommandAllocator.Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&commandList)));
//
//    // Create the vertex buffer.
//    {
//        // Define the geometry for a triangle.
//        Vertex triangleVertices[] =
//        {
//            { { -1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
//            { { 1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
//            { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
//        };
//        const UINT vertexBufferSize = sizeof(triangleVertices);
//
//        // We create a default and upload buffer. Using the upload buffer, we transfer the data from the CPU to the GPU (hence the name) but we do not use the buffer as reference.
//        // We copy the data from our upload buffer to the default buffer, and the only differenc between the two is the staging - Upload vs Default.
//        // Default types are best for static data that isn't changing.
//        WRL::ComPtr<ID3D12Resource> uploadBuffer;
//        m_vertexBuffer = CreateDefaultBuffer(m_pDevice, commandList, triangleVertices,
//            sizeof(Vertex) * 3, uploadBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, L"default vb");
//
//        // We must wait and insure the data has been copied before moving on 
//        // After we execute the command list, we need to sync with the GPU and wait to create our buffer view
//        ThrowIfFailed(commandList->Close());
//
//        // Initialize the vertex buffer view.
//        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
//        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
//        m_vertexBufferView.SizeInBytes = vertexBufferSize;
//
//        // Execute both commands lists before moving on
//        ID3D12CommandList* ppCommandList[] = { commandList.Get() };
//        g_pCommandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);
//
//        // Bundle Test - The vertex buffer isn't iniitialized until here, and we are still in recording state from LoadAssets() call
//        // So now we can just fulfill our commands and close it. 
//        {
//            m_pBundleList->SetGraphicsRootSignature(m_pRootSignature.Get());
//            m_pBundleList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//            m_pBundleList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
//            m_pBundleList->DrawInstanced(3, 1, 0, 0);
//            ThrowIfFailed(m_pBundleList->Close());
//        }
//        // Wait for GPU work to finish
//        {
//            ThrowIfFailed(m_pDevice->CreateFence(m_fenceLastSignaledValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
//            ++m_fenceLastSignaledValue;
//
//            m_fenceEvent = CreateEvent(nullptr, false, false, L"Fence Event 2");
//
//            if (m_fenceEvent == nullptr)
//            {
//                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
//            }
//
//            const auto waitForValue = m_fenceLastSignaledValue;
//            ThrowIfFailed(g_pCommandQueue->Signal(m_fence.Get(), waitForValue));
//            ++m_fenceLastSignaledValue;
//
//            ThrowIfFailed(m_fence->SetEventOnCompletion(waitForValue, m_fenceEvent));
//            WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
//
//        }
//    }
//}

void gt::dx12::DX12CubeApp::CreateCommandQueue()
{
    auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    // TODO: Create an "Initialize" state to pass the device and setup these objects
    // instead of just in the default constructor
    WRL::ComPtr<ID3D12Device4> device4;
    auto hr = m_device->GetDevice().As(&device4);
    LS::Utils::ThrowIfFailed(hr, "Failed to obtain ID3D12Device4 interface");
    auto errorCode = m_directQueue.Initialize(device4.Get());
    if (!errorCode)
    {
        LS::Utils::ThrowIfFailed(E_FAIL, errorCode.Message());
    }

    errorCode = m_copyQueue.Initialize(device4.Get());
    if (!errorCode)
    {
        LS::Utils::ThrowIfFailed(E_FAIL, errorCode.Message());
    }

    m_commandList = std::make_unique<LS::Platform::Dx12::CommandListDx12>(device4.Get(), type, "main command list");
}

void gt::dx12::DX12CubeApp::CreateSwapchain()
{
    const auto& window = Window;
    HWND hwnd = reinterpret_cast<HWND>(window->GetHandleToWindow());

    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = FRAME_COUNT;
    swapchainDesc1.Width = window->GetWidth();
    swapchainDesc1.Height = window->GetHeight();
    swapchainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    swapchainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc1.SampleDesc.Count = 1;
    swapchainDesc1.SampleDesc.Quality = 0;
    swapchainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc1.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc1.Stereo = FALSE;

    auto result = m_frameBuffer.InitializeFrameBuffer(m_pFactory, m_directQueue.GetCommandQueue().Get(), hwnd, swapchainDesc1);
    if (!result)
    {
        LS::Utils::ThrowIfFailed(E_FAIL, "Failed to initialize the frame buffer");
    }

    // Helper function that displays our display's resolution and refresh rates and other information 
    LS::Win32::LogAdapters(m_pFactory.Get());

    // Don't allot ALT+ENTER fullscreen
    m_pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
}

void gt::dx12::DX12CubeApp::CreateDescriptors()
{
    // Descriptor - a block of data that describes an object to the GPU (SRV, UAVs, CBVs, RTVs, DSVs)
    // Descriptor Heap - A collection of contiguous allocations of descriptors

     // This is the RTV descriptor heap (render target view)
    {
        m_heapRtv.Initialize(m_device->GetDevice().Get());
    }

    // Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    {
        m_heapSrv.Initialize(m_device->GetDevice().Get());
    }

    // Depth Stencil View 
    {
        m_heapDsv.Initialize(m_device->GetDevice().Get());
    }
}

//void gt::dx12::Draw(D3D12_VERTEX_BUFFER_VIEW& bufferView, uint64_t vertices, uint64_t instances)
//{
//    auto fc = &m_frameContexts[m_frameResourceIndex];
//    auto& commandlist = fc->CommandList;
//    commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    commandlist->IASetVertexBuffers(0, 1, &bufferView);
//    commandlist->DrawInstanced((UINT)vertices, (UINT)instances, 0, 0);
//}

void gt::dx12::DX12CubeApp::SetupState()
{
    //auto commandList = m_commandList->GetCommandList();
    auto frame = m_frameBuffer.GetFramePtr();
    const std::array<float, 4> color{ 0.4f, 0.5f, 0.9f, 1.0f };
    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_heapDsv.GetHeapStartCpu();
    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = frame->GetDescriptorHandle();

    m_commandList->SetVertexBuffers(0, 1, &m_cubeVbView);
    m_commandList->SetIndexBuffer(&m_cubeIbView);
    m_commandList->SetViewports(1, &m_viewport);
    m_commandList->SetScissorRects(1, &m_scissorRect);

    m_commandList->Clear(color, rtv);
    m_commandList->ClearDepthStencil(dsv);
    m_commandList->SetPipelineState(m_cubePipelineState.Get());
    m_commandList->SetGraphicsRootSignature(m_cubeRootSignature.Get());
    m_commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->SetRenderTarget(rtv, &dsv);
}

void gt::dx12::DX12CubeApp::Update()
{
    XMMATRIX mvpMatrix = XMMatrixMultiply(m_cubeModelMat, m_cubeViewMat);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, m_cubeProjMat);

    m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);
    m_commandList->DrawIndexedInstanced(_countof(g_indices), 1);
}

auto gt::dx12::DX12CubeApp::Initialize(int argC, char* argsV[]) -> LS::System::ErrorCode
{
    if (!CreateDevice())
        return LS::System::CreateFailCode("Failed to create device.");

    CreateCommandQueue();
    CreateDescriptors();
    CreateSwapchain();
    LoadContent();

    // Create View Port //
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Window->GetWidth()), static_cast<float>(Window->GetHeight()));
    // Scissor Rect is the actual drawing area of what will be rendered. A viewport can be bigger than the scissor rect,
    // or you can use Scissor rects to specify specific regions to draw (like omitting UI areas that may never be drawn because 2D render systems would handle that)
    m_scissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
    g_aspectRatio = m_viewport.Width / m_viewport.Height;
    m_fov = 45.0f;

    m_contentLoaded = true;
    return LS::System::CreateSuccessCode();
}

void gt::dx12::DX12CubeApp::Run()
{
    IsRunning = true;
    Window->Show();
    /*while (Window->IsOpen())
    {
        Window->PollEvent();
        Render(0.20f, 0.38f, 0.65f, 1.0f);
    }*/
    DemoRun();
}

void gt::dx12::DX12CubeApp::UpdateBufferResource(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;
    // Create a committed resource for the GPU int he default heap
    const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto defaultBuffDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
    auto device = m_device->GetDevice();
    const auto hr = device->CreateCommittedResource(&defaultHeap, 
        D3D12_HEAP_FLAG_NONE, &defaultBuffDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(pDestinationResource));

    LS::Utils::ThrowIfFailed(hr, "Failed to commit resource onto the GPU");

    // Create the upload buffer 
    if (bufferData)
    {
        const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        const auto uploadHr = device->CreateCommittedResource(&uploadHeap, 
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, 
            IID_PPV_ARGS(pIntermediateResource));

        LS::Utils::ThrowIfFailed(uploadHr, "Failed to create upload buffer");

        // Perform data transfer 
        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}

bool gt::dx12::DX12CubeApp::LoadContent()
{
    // Upload vertex buffer data //
    auto commandList = m_commandList->GetCommandList();
    Microsoft::WRL::ComPtr<ID3D12Device4> device4;
    auto hr = m_device->GetDevice().As(&device4);
    if (FAILED(hr))
    {
        return false;
    }
    WRL::ComPtr<ID3D12Resource> intermediateVB;
    UpdateBufferResource(commandList, &m_cubeVb,
        &intermediateVB, _countof(g_vertices), sizeof(VertexPositionColor), g_vertices);

    m_cubeVbView.BufferLocation = m_cubeVb->GetGPUVirtualAddress();
    m_cubeVbView.SizeInBytes = sizeof(g_vertices);
    m_cubeVbView.StrideInBytes = sizeof(VertexPositionColor);

    // Upload the index buffer // 
    WRL::ComPtr<ID3D12Resource> intermediateIB;
    UpdateBufferResource(commandList, &m_cubeIb, &intermediateIB, _countof(g_indices), sizeof(WORD), g_indices);

    m_cubeIbView.BufferLocation = m_cubeIb->GetGPUVirtualAddress();
    m_cubeIbView.Format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
    m_cubeIbView.SizeInBytes = sizeof(g_indices);

    // Find current directory to get our path and add the shaders (they'll be local to the exe for now)
    std::array<wchar_t, _MAX_PATH> modulePath{};
    if (!GetModuleFileName(nullptr, modulePath.data(), 
        static_cast<DWORD>(modulePath.size())))
    {
        ThrowIfFailed(ERROR_FILE_NOT_FOUND);
    }

    std::wstring path = std::wstring(modulePath.data());
    const auto fsPath = std::filesystem::path(path);
    const auto parent = fsPath.parent_path();
    std::wstring vsPath = std::format(L"{}\\VertexShaderD3D12.cso", parent.wstring());
    std::wstring psPath = std::format(L"{}\\PixelShaderD3D12.cso", parent.wstring());
    // Load the shaders for this project
    WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(vsPath.c_str(), &vertexShaderBlob));

    WRL::ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(psPath.c_str(), &pixelShaderBlob));

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {.SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 },
        {.SemanticName = "COLOR", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device4->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // Serialize the root signature.
    WRL::ComPtr<ID3DBlob> rootSignatureBlob;
    WRL::ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    // Create the root signature.
    ThrowIfFailed(device4->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_cubeRootSignature)));

    // Number of RTVs //
    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Create the graphics pipeline state object (PSO)
    g_pipelineStateStream.pRootSignature = m_cubeRootSignature.Get();
    g_pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    g_pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    g_pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    g_pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    g_pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    g_pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { sizeof(PipelineStateStream), &g_pipelineStateStream };
    ThrowIfFailed(device4->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_cubePipelineState)));

    //D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    //psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    //psoDesc.pRootSignature = m_cubeRootSignature.Get();
    //psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    //psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    //psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    //psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    //psoDesc.DepthStencilState.DepthEnable = TRUE;
    //psoDesc.DepthStencilState.StencilEnable = TRUE;
    //psoDesc.SampleMask = UINT_MAX;
    //psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //psoDesc.NumRenderTargets = 1;
    //psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    //psoDesc.SampleDesc.Count = 1;
    //ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState)));

    // Execute to insure the index and vertex buffers are uploaded to the GPU resources before rendering
    commandList->Close();
    m_copyQueue.QueueCommand(m_commandList.get());
    auto fenceValue = m_copyQueue.ExecuteCommandList();
    m_copyQueue.WaitForGpu(fenceValue);

    m_contentLoaded = true;
    // Resize/Create the depth buffer.
    ResizeDepthBuffer(Window->GetWidth(), Window->GetHeight());

    return true;
}

void gt::dx12::DX12CubeApp::DemoRun()
{
    const auto begin = std::chrono::high_resolution_clock::now();
    //UINT64 fenceValue = 0;
    while (Window->IsOpen())
    {
        Window->PollEvent();
        const auto tick = std::chrono::high_resolution_clock::now();
        const auto end = std::chrono::high_resolution_clock::now();

        const auto delta = end - tick;
        const auto total = end - begin;
        float angle = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
        const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
        m_cubeModelMat = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

        const XMVECTOR eyePos = XMVectorSet(0, 0, -10, 1);
        const XMVECTOR focus = XMVectorSet(0, 0, 0, 1);
        const XMVECTOR up = XMVectorSet(0, 1, 0, 1);

        m_cubeViewMat = XMMatrixLookAtLH(eyePos, focus, up);

        float aspectRatio = Window->GetWidth() / static_cast<float>(Window->GetHeight());
        m_cubeProjMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), aspectRatio, 0.1f, 100.0f);

        m_commandList->ResetCommandList();

        SetupState();
        Update();

        m_commandList->Close();

        // Render Work // 
        //m_frameResourceIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        //const auto fc = &m_frameContexts[m_frameResourceIndex];
        /*const auto commandList = m_frameContexts[m_frameResourceIndex].CommandList;
        const auto commandAllocator = m_frameContexts[m_frameResourceIndex].CommandAllocator;*/
        //ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));

        //ClearRTV(0.4f, 0.5f, 0.9f, 1.0f);
        //ClearDepth(1.0f);
        //SetupState();
        //Update();

        // My local usage of things // 
        //ThrowIfFailed(commandList->Close());

        //ID3D12CommandList* const ppCommands[] = { commandList.Get() };
        //m_pDirectCommandQueue->ExecuteCommandLists(1, ppCommands);

        /*DXGI_PRESENT_PARAMETERS params{};
        params.DirtyRectsCount = 0;
        params.pDirtyRects = nullptr;
        params.pScrollOffset = nullptr;
        params.pScrollRect = nullptr;

        ThrowIfFailed(m_pSwapChain->Present1(0, 0, &params));*/
        //ThrowIfFailed(m_pSwapChain->Present(1, 0));

        // Wait for GPU // 
        //auto fenceValue = m_fenceValues[m_frameResourceIndex];
        //ThrowIfFailed(m_pDirectCommandQueue->Signal(m_fence.Get(), fenceValue));
        //m_fenceValues[m_frameResourceIndex]++;

        //if (m_fence->GetCompletedValue() < fenceValue)
        //{
        //    ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
        //    ::WaitForSingleObject(m_fenceEvent, INFINITE);
        //}
    }
}

void gt::dx12::DX12CubeApp::ResizeDepthBuffer(int width, int height)
{
    if (m_contentLoaded)
    {
        m_directQueue.Flush();
        m_copyQueue.Flush();
        auto device = m_device->GetDevice();

        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        const CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
        const auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        ThrowIfFailed(device->CreateCommittedResource(
            &heap,
            D3D12_HEAP_FLAG_NONE,
            &resDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&m_depthBuffer)
        ));

        // Update the depth-stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_depthBuffer.Get(), &dsv,
            m_heapDsv.GetHeapStartCpu());
    }
}

//void gt::dx12::DX12CubeApp::ClearDepth(float depth)
//{
//    auto fc = &m_frameContexts[m_frameResourceIndex];
//    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
//    fc->CommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0xFF, 0, nullptr);
//}

void gt::dx12::DX12CubeApp::Render(float r, float g, float b, float a)
{
}

void gt::dx12::DX12CubeApp::OnDestroy()
{
    m_frameBuffer.WaitOnFrameBuffer();
    m_directQueue.Flush();
    m_copyQueue.Flush();
}