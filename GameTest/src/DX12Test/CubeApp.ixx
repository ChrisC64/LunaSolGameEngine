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
export module CubeApp;

import LSData;
import Engine.App;
import D3D12Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Helper.PipelineFactory;
import Util.MSUtils;

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
        // Manages a heap for the command lists. This cannot be reset while the CommandList is still in flight on the GPU0
        ComPtrArray<ID3D12CommandAllocator, NUM_CONTEXT> CommandAllocator;
        // Use with the bundle list, this allocator performs the same operations as a command list, but is associated with the bundle
        WRL::ComPtr<ID3D12CommandAllocator> BundleAllocator;
        // Sends commands to the GPU - represents this frames commands
        ComPtrArray<ID3D12GraphicsCommandList, NUM_CONTEXT> CommandList;
        // Bundle up calls you would want repeated constantly, like setting up a draw for a vertex buffer. 
        WRL::ComPtr<ID3D12GraphicsCommandList> BundleList;
        // Singal value between the GPU and CPU to perform synchronization. 
        UINT64                         FenceValue;
    };

    export class DX12CubeApp : public LS::LSApp
    {
    public:
        DX12CubeApp() = default;
        DX12CubeApp(uint32_t width, uint32_t height, std::wstring_view title) 
        {
            Window = LS::BuildWindow(width, height, title);
        }
        ~DX12CubeApp() = default;

        auto Initialize(int argCount = 0, char* argsV[] = nullptr) -> LS::System::ErrorCode override;
        void Run() override;

    private:
        // Tutorial Stuff //
        WRL::ComPtr<ID3D12Device4> m_pDevice;

        WRL::ComPtr<ID3D12Resource> m_cubeVb;
        D3D12_VERTEX_BUFFER_VIEW m_cubeVbView;

        WRL::ComPtr<ID3D12Resource> m_cubeIb;
        D3D12_INDEX_BUFFER_VIEW m_cubeIbView;

        WRL::ComPtr<ID3D12Resource> m_depthBuffer;
        WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        std::array<uint64_t, 3> m_fenceValues;
        WRL::ComPtr<ID3D12RootSignature> m_cubeRootSignature;
        WRL::ComPtr<ID3D12PipelineState> m_cubePipelineState;
        WRL::ComPtr<IDXGIFactory7> m_factory;
        float m_fov;
        XMMATRIX m_cubeModelMat;
        XMMATRIX m_cubeViewMat;
        XMMATRIX m_cubeProjMat;
        bool m_contentLoaded;
        Ref<LS::Platform::Dx12::CommandQueue> m_copyCommandQueue, m_directCommandQueue;

        bool CreateDevice(HWND hwnd, uint32_t x, uint32_t y);
        void CreateRenderTarget();
        void CreateSwapchain();
        void CreateCommandAllocators();
        void CreateDescriptors();
        bool LoadAssets();
        void LoadVertexDataToGpu();
        void UpdateBufferResource(WRL::ComPtr<ID3D12GraphicsCommandList>& commandLIst,
            ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
            size_t numElements, size_t elementSize, const void* bufferData,
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
        bool LoadContent();
        void ResizeDepthBuffer(int width, int height);
        void TransitionResource(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, WRL::ComPtr<ID3D12Resource>& resource,
            D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
        void ClearRTV(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, float* clearColor);
        void ClearDepth(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, float depth);
        // END Tutorial Stuff //
    };

    struct ThreadParameter
    {
        uint32_t threadIndex;
    };

    ThreadParameter m_threadParameters[NUM_CONTEXT];

    std::array<FrameContext, FRAME_COUNT>					m_frameContext = {};
    FrameContext*                                           m_pCurrFrameContext;
    uint32_t												m_frameResourceIndex;
    uint32_t												g_frameIndex;
    float													g_aspectRatio;

    // pipeline objects
    
    WRL::ComPtr<ID3D12DescriptorHeap>						g_pRtvDescHeap;
    WRL::ComPtr<ID3D12DescriptorHeap>						g_pSrvDescHeap;
    WRL::ComPtr<ID3D12CommandAllocator>						m_pCommandAllocator; // local command allocator for non-frame resource related jobs.
    WRL::ComPtr<ID3D12CommandAllocator>						m_pCommandAllocator2; // local command allocator for non-frame resource related jobs.
    WRL::ComPtr<ID3D12CommandQueue>							m_pCommandQueue;
    WRL::ComPtr<IDXGISwapChain4>							g_pSwapChain = nullptr;
    //ComPtr<ID3D12GraphicsCommandList>						m_pCommandList; // Records drawing or state chaning calls for execution later by the GPU - Set states, draw calls - think the D3D11::ImmediateContext 
    WRL::ComPtr<ID3D12CommandAllocator>						m_pBundleAllocator;
    WRL::ComPtr<ID3D12GraphicsCommandList>					m_pBundleList;
    WRL::ComPtr<ID3D12RootSignature>						m_pRootSignature; // Used with shaders to determine input and variables
    WRL::ComPtr<ID3D12RootSignature>						m_pRootSignature2; // Used with shaders to determine input and variables - texture_effect.hlsl
    WRL::ComPtr<ID3D12PipelineState>						m_pPipelineState; // Defines our pipeline's state - primitive topology, render targets, shaders, etc. 
    WRL::ComPtr<ID3D12PipelineState>						m_pPipelineStatePT; // Defines our pipeline's state - primitive topology, render targets, shaders, etc. 
    HANDLE													g_scWaitableHandle = nullptr;
    std::array<WRL::ComPtr<ID3D12Resource>, FRAME_COUNT>	g_backBuffers = {};// Our Render Target resources
    D3D12_CPU_DESCRIPTOR_HANDLE								g_rtvDescHandle[FRAME_COUNT] = {};
    CD3DX12_VIEWPORT										g_viewport;
    CD3DX12_RECT											g_scissorRect;

    // App resources
    WRL::ComPtr<ID3D12Resource>								m_vertexBuffer = nullptr;
    WRL::ComPtr<ID3D12Resource>								m_vertexBufferPT = nullptr;
    //ComPtr<ID3D12Resource>								m_uploadBuffer = nullptr;
    WRL::ComPtr<ID3D12Resource>								m_texture = nullptr;
    D3D12_VERTEX_BUFFER_VIEW								m_vertexBufferView;
    D3D12_VERTEX_BUFFER_VIEW								m_vertexBufferViewPT;
    // Synchronization Objects
    WRL::ComPtr<ID3D12Fence>								m_fence;// Helps us sync between the GPU and CPU
    HANDLE													m_fenceEvent = nullptr;
    uint64_t												m_fenceLastSignaledValue = 0;
    UINT													g_rtvDescriptorSize = 0;
    HANDLE													g_drawFinished;
    HANDLE													g_prepWorkDone;
    HANDLE													g_beginRender;


    //std::array<std::thread, Engine::NUM_CONTEXT>			m_threadPool;
    std::jthread											m_defaultDrawThread;
    std::jthread											m_textureDrawThread;
    // Blah stuff
    bool firstRun = true;

    void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter);
    void CheckFeatures([[maybe_unused]] std::string& s);
    void LogAdapters(IDXGIFactory4* factory);
    void LogAdapterOutput(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
    void ExecuteCommandList();
    void WaitForGpu();
    void BeginRender();
    void ResetCommandList();
    void SetPipelineState(WRL::ComPtr<ID3D12PipelineState>& pipelineState);
    void SetRootSignature(WRL::ComPtr<ID3D12RootSignature>& rootSignature);
    void SetSrvDescHeap();
    void SetViewport();
    void ClearRTV(float r, float g, float b, float a);
    void SetRTV(FrameContext* frameCon);
    void Draw(D3D12_VERTEX_BUFFER_VIEW& bufferView, uint64_t vertices, uint64_t instances = 1u);
    void TansitionRtvToPresent();
    void CloseCommandList();
    void MoveToNextFrame();
    void CreateThreads();
    void Render(float r, float g, float b, float a);
    void OnDestroy();

    inline auto CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, uint64_t byteSize, WRL::ComPtr<ID3D12Resource>& uploadBuffer, D3D12_RESOURCE_STATES finalState, std::optional<std::wstring_view> defaultName = std::nullopt, std::optional<std::wstring_view> uploadName = std::nullopt) -> WRL::ComPtr<ID3D12Resource>;
    inline auto CreateDefaultBuffer2(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, uint64_t byteSize, WRL::ComPtr<ID3D12Resource>& uploadBuffer, D3D12_RESOURCE_STATES finalState) -> WRL::ComPtr<ID3D12Resource>;
    inline auto GenerateTextureData(uint32_t textureWidth, uint32_t textureHeight, uint32_t pixelSize) -> std::vector<UINT8>;
    void CreateTileSampleTexture(ID3D12Device* device, WRL::ComPtr<ID3D12Resource>& texture, uint32_t textureWidth, uint32_t textureHeight, uint32_t pixelSize, WRL::ComPtr<ID3D12Resource>& textureUploadHeap, WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, WRL::ComPtr<ID3D12DescriptorHeap>& srvHeap);
}

module : private;

using namespace gt;

inline auto gt::dx12::CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
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

    auto defaultBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1,
        &defaultBarrier);

    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subresourceData);

    auto defaultBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, finalState);
    cmdList->ResourceBarrier(1,
        &defaultBarrier2);

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

inline auto gt::dx12::CreateDefaultBuffer2(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, uint64_t byteSize, WRL::ComPtr<ID3D12Resource>& uploadBuffer, D3D12_RESOURCE_STATES finalState) -> WRL::ComPtr<ID3D12Resource>
{
    WRL::ComPtr<ID3D12Resource> defaultBuffer;

    // Creat Default Buffer
    auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapDefault,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())
    ));

    // Upload type is needed to get the data onto the GPU
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

    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subresourceData);

    auto defaultBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, finalState);
    cmdList->ResourceBarrier(1,
        &defaultBarrier2);

    return defaultBuffer;
}

inline auto gt::dx12::GenerateTextureData(uint32_t textureWidth, uint32_t textureHeight, uint32_t pixelSize) -> std::vector<UINT8>
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

void gt::dx12::CreateTileSampleTexture(ID3D12Device* device, WRL::ComPtr<ID3D12Resource>& texture, uint32_t textureWidth, uint32_t textureHeight, uint32_t pixelSize, WRL::ComPtr<ID3D12Resource>& textureUploadHeap, WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, WRL::ComPtr<ID3D12DescriptorHeap>& srvHeap)
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

    UpdateSubresources(commandList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureSubresourceData);
    auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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

bool gt::dx12::DX12CubeApp::CreateDevice(HWND hwnd, uint32_t x, uint32_t y)
{
    // [DEBUG] Enable debug interface
#ifdef _DEBUG
    WRL::ComPtr<ID3D12Debug> pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif
    // Create our DXGI Factory
    CreateDXGIFactory2(0u, IID_PPV_ARGS(&m_factory));

    // Find the best graphics card (best performing one, with single GPU systems, this should be the default)
    WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(m_factory.Get(), &hardwareAdapter, false);

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), featureLevel, IID_PPV_ARGS(&m_pDevice)));

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef _DEBUG
    if (pdx12Debug)
    {
        WRL::ComPtr<ID3D12InfoQueue> pInfoQueue = nullptr;
        m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    }
#endif
    return true;
}

bool gt::dx12::DX12CubeApp::LoadAssets()
{
    // Create an empty root signature for shader.hlsl
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        WRL::ComPtr<ID3DBlob> signature;
        WRL::ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature)));
    }

    // Create a root signature for our texture_effect.hlsl
    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        WRL::ComPtr<ID3DBlob> signature;
        WRL::ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature2)));
    }
    // Create pipeline states and associate to command allocators since we have an array of them
    for (auto& fc : m_frameContext)
    {
        for (auto i = 0u; i < NUM_CONTEXT; ++i)
        {
            ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&fc.CommandAllocator[i])));
            ThrowIfFailed(m_pDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&fc.CommandList[i])));
            fc.CommandAllocator[i]->SetName(L"FC Command Allocator " + i);
            fc.CommandList[i]->SetName(L"FC Command List " + i);
        }
    }

    m_pCurrFrameContext = &m_frameContext[g_frameIndex];
    // Create the pipeline state, which includes compiling and loading shaders.
    {
        WRL::ComPtr<ID3DBlob> vertexShader;
        WRL::ComPtr<ID3DBlob> pixelShader;
        WRL::ComPtr<ID3DBlob> vertexShader2;
        WRL::ComPtr<ID3DBlob> pixelShader2;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        //TODO: Create another PSO that uses the root signature and input element descriptions for our second textured triangle
        // then see if we can display both on the scene at once. 
        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_pRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState)));
        // Bundle Test // 
        {
            ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_pBundleAllocator)));
            ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pBundleAllocator.Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&m_pBundleList)));
        }

        // Create root signature for texture_effect.hlsl
        ThrowIfFailed(D3DCompileFromFile(L"texture_effect.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader2, nullptr));
        ThrowIfFailed(D3DCompileFromFile(L"texture_effect.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader2, nullptr));
        D3D12_INPUT_ELEMENT_DESC inputElementDescs2[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        psoDesc.InputLayout = { inputElementDescs2, _countof(inputElementDescs2) };
        psoDesc.pRootSignature = m_pRootSignature2.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader2.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader2.Get());
        ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineStatePT)));
    }

    {
        // A fence is used for synchronization
        if (m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)) != S_OK)
            return false;

        // Update the fence value, from startup, this should be 0, and thus the next frame we'll be creating will be the first frame (back buffer, as 0 is currently in front)
        //m_frameContext[m_frameResourceIndex].FenceValue++;
        m_pCurrFrameContext->FenceValue++;

        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, L"Fence Event");
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    return true;
}

void gt::dx12::DX12CubeApp::LoadVertexDataToGpu()
{
    // For simplicity, I've opted to make two command lists with two separate local command allocators. This is so I can then put these two command lists to work
    // right away, without waiting on one or the other. These jobs only happen once, so it's only done this way for quick and easy, over "correctness" of loading data onto the GPU.
    WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), m_pPipelineState.Get(), IID_PPV_ARGS(&commandList)));

    WRL::ComPtr<ID3D12GraphicsCommandList> commandList2;
    ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator2.Get(), m_pPipelineStatePT.Get(), IID_PPV_ARGS(&commandList2)));

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { -1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };
        const UINT vertexBufferSize = sizeof(triangleVertices);

        // We create a default and upload buffer. Using the upload buffer, we transfer the data from the CPU to the GPU (hence the name) but we do not use the buffer as reference.
        // We copy the data from our upload buffer to the default buffer, and the only differenc between the two is the staging - Upload vs Default.
        // Default types are best for static data that isn't changing.
        WRL::ComPtr<ID3D12Resource> uploadBuffer;
        m_vertexBuffer = CreateDefaultBuffer(m_pDevice.Get(), commandList.Get(), triangleVertices,
            sizeof(Vertex) * 3, uploadBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, L"default vb");

        // We must wait and insure the data has been copied before moving on 
        // After we execute the command list, we need to sync with the GPU and wait to create our buffer view
        ThrowIfFailed(commandList->Close());

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;

        // Textured Triangle
        VertexPT triangleVerticesPT[] =
        {
            { { 0.0f, 0.25f * g_aspectRatio, 0.0f }, { 0.5f, 0.0f } },
            { { 0.25f, -0.25f * g_aspectRatio, 0.0f }, { 1.0f, 1.0f } },
            { { -0.25f, -0.25f * g_aspectRatio, 0.0f }, { 0.0f, 1.0f } }
        };
        const UINT vertexBufferSize2 = sizeof(triangleVerticesPT);

        WRL::ComPtr<ID3D12Resource> textureUploadBuffer;
        m_vertexBufferPT = CreateDefaultBuffer(m_pDevice.Get(), commandList2.Get(), triangleVerticesPT,
            sizeof(VertexPT) * 3, textureUploadBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, L"pt default vb");
        WRL::ComPtr<ID3D12Resource> textureUploadHeap;
        {
            CreateTileSampleTexture(m_pDevice.Get(), m_texture, 256u, 256u, 4u, textureUploadHeap, commandList2, g_pSrvDescHeap);
        }

        // Execute both commands lists before moving on
        commandList2->Close();
        ID3D12CommandList* ppCommandList[] = { commandList.Get(), commandList2.Get() };
        m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

        // Initialize the vertex buffer view.
        m_vertexBufferViewPT.BufferLocation = m_vertexBufferPT->GetGPUVirtualAddress();
        m_vertexBufferViewPT.StrideInBytes = sizeof(VertexPT);
        m_vertexBufferViewPT.SizeInBytes = vertexBufferSize2;

        // Bundle Test - The vertex buffer isn't iniitialized until here, and we are still in recording state from LoadAssets() call
        // So now we can just fulfill our commands and close it. 
        {
            m_pBundleList->SetGraphicsRootSignature(m_pRootSignature.Get());
            m_pBundleList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_pBundleList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
            m_pBundleList->DrawInstanced(3, 1, 0, 0);
            ThrowIfFailed(m_pBundleList->Close());
        }
        // Wait for GPU work to finish
        {
            ThrowIfFailed(m_pDevice->CreateFence(m_fenceLastSignaledValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
            ++m_fenceLastSignaledValue;

            m_fenceEvent = CreateEvent(nullptr, false, false, L"Fence Event 2");

            if (m_fenceEvent == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }

            const auto waitForValue = m_fenceLastSignaledValue;
            ThrowIfFailed(m_pCommandQueue->Signal(m_fence.Get(), waitForValue));
            ++m_fenceLastSignaledValue;

            ThrowIfFailed(m_fence->SetEventOnCompletion(waitForValue, m_fenceEvent));
            WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

        }
    }
}

void gt::dx12::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    WRL::ComPtr<IDXGIAdapter1> adapter;

    WRL::ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

void gt::dx12::DX12CubeApp::CreateRenderTarget()
{
    // The handle can now be used to help use build our RTVs - one RTV per frame/back buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        ThrowIfFailed(g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_backBuffers[i])));
        m_pDevice->CreateRenderTargetView(g_backBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, g_rtvDescriptorSize);
        //rtvHandle.Offset(g_rtvDescriptorSize);
        g_rtvDescHandle[i] = rtvHandle;
    }
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

    // Since we are using an HWND (Win32) system, we can create the swapchain for HWND 
    {
        WRL::ComPtr<IDXGISwapChain1> swapChain1 = nullptr;
        //ThrowIfFailed(m_factory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hwnd, &swapchainDesc1, nullptr, nullptr, &swapChain1));
        ThrowIfFailed(m_factory->CreateSwapChainForHwnd(m_directCommandQueue->GetCommandQueue().Get(), hwnd, &swapchainDesc1, nullptr, nullptr, &swapChain1));
        ThrowIfFailed(swapChain1.As(&g_pSwapChain));

        // Helper function that displays our display's resolution and refresh rates and other information 
        LogAdapters(m_factory.Get());
        // Don't allot ALT+ENTER fullscreen
        m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        g_pSwapChain->SetMaximumFrameLatency(FRAME_COUNT);
        g_frameIndex = g_pSwapChain->GetCurrentBackBufferIndex();
        g_scWaitableHandle = g_pSwapChain->GetFrameLatencyWaitableObject();
    }
}

void gt::dx12::DX12CubeApp::CreateCommandAllocators()
{
    ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)));
    m_pCommandAllocator->SetName(L"Data Command Allocator");
    ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator2)));
    m_pCommandAllocator2->SetName(L"Data Command Allocator 2");

    // Create Command Queues //
    m_copyCommandQueue = std::make_unique<LS::Platform::Dx12::CommandQueue>(m_pDevice, D3D12_COMMAND_LIST_TYPE_COPY);
    m_directCommandQueue = std::make_unique<LS::Platform::Dx12::CommandQueue>(m_pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);

}

void gt::dx12::DX12CubeApp::CreateDescriptors()
{
    // Descriptor - a block of data that describes an object to the GPU (SRV, UAVs, CBVs, RTVs, DSVs)
    // Descriptor Heap - A collection of contiguous allocations of descriptors
    // This is the RTV descriptor heap (render target view)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NumDescriptors = FRAME_COUNT;
        //desc.NodeMask = 1;

        ThrowIfFailed((m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pRtvDescHeap))));
        // Handles have a size that varies by GPU, so we have to ask for the Handle size on the GPU before processing
        g_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // a descriptor heap for the Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NumDescriptors = 1;

        ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pSrvDescHeap)));
    }
}

void gt::dx12::CheckFeatures([[maybe_unused]] std::string& s)
{
}

void gt::dx12::LogAdapters(IDXGIFactory4* factory)
{
    uint32_t i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        std::wstring text = L"***Adapter: ";
        text += desc.Description;
        text += L"\n";
#ifdef _DEBUG
        OutputDebugString(text.c_str());
#else
        std::wcout << text << L"\n";
#endif
        adapterList.emplace_back(adapter);
        ++i;
    }

    for (auto a : adapterList)
    {
        LogAdapterOutput(a);
        a->Release();
    }
}

void gt::dx12::LogAdapterOutput(IDXGIAdapter* adapter)
{
    uint32_t i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";
#ifdef _DEBUG
        OutputDebugString(text.c_str());
#else
        std::wcout << text << L"\n";
#endif
        LogOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);

        output->Release();
        ++i;
    }
}

void gt::dx12::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    output->GetDisplayModeList(format, flags, &count, nullptr);
    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(x.Width) + L" " +
            L"Height = " + std::to_wstring(x.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) + L"\n";
#ifdef _DEBUG
        OutputDebugString(text.c_str());
#else
        std::wcout << text << L"\n";
#endif
    }
}

void gt::dx12::ExecuteCommandList()
{
    // Execut the command list
    /*ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);*/
    ID3D12CommandList* ppCommandLists[] = { m_pCurrFrameContext->CommandList[m_frameResourceIndex].Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

// Waits for work on the GPU to finish before moving on to the next frame
void gt::dx12::WaitForGpu()
{
    FrameContext* frameCon = &m_frameContext[m_frameResourceIndex];

    // Signals the GPU the next upcoming fence value
    ThrowIfFailed(m_pCommandQueue->Signal(m_fence.Get(), frameCon->FenceValue));

    // Wait for the fence to be processes
    ThrowIfFailed(m_fence->SetEventOnCompletion(frameCon->FenceValue, m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    // Increment value to the next frame
    frameCon->FenceValue++;
}

void gt::dx12::BeginRender()
{
    m_pCurrFrameContext = &m_frameContext[m_frameResourceIndex];
    // Reclaims the memory allocated by this allocator for our next usage
    ThrowIfFailed(m_pCurrFrameContext->CommandAllocator[m_frameResourceIndex]->Reset());
}

void gt::dx12::ResetCommandList()
{
    // Resets a command list to its initial state 
    ThrowIfFailed(m_pCurrFrameContext->CommandList[m_frameResourceIndex]->Reset(m_pCurrFrameContext->CommandAllocator[m_frameResourceIndex].Get(), nullptr));
}

void gt::dx12::SetPipelineState(WRL::ComPtr<ID3D12PipelineState>& pipelineState)
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetPipelineState(pipelineState.Get());
}

void gt::dx12::SetRootSignature(WRL::ComPtr<ID3D12RootSignature>& rootSignature)
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetGraphicsRootSignature(rootSignature.Get());
}

void gt::dx12::SetSrvDescHeap()
{
    ID3D12DescriptorHeap* ppHeaps[] = { g_pSrvDescHeap.Get() };
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetGraphicsRootDescriptorTable(0, g_pSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
}

void gt::dx12::SetViewport()
{
    // Set Viewport
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->RSSetViewports(1, &g_viewport);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->RSSetScissorRects(1, &g_scissorRect);
}

void gt::dx12::ClearRTV(float r, float g, float b, float a)
{
    // This will prep the back buffer as our render target and prepare it for transition
    auto backbufferIndex = g_pSwapChain->GetCurrentBackBufferIndex();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(g_backBuffers[backbufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart(), m_frameResourceIndex, g_rtvDescriptorSize);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    // Similar to D3D11 - this is our command for drawing. For now, testing triangle drawing through MSDN example code
    const float color[] = { r, g, b, a };
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

}

void gt::dx12::SetRTV(FrameContext* frameCon)
{
    // This will prep the back buffer as our render target and prepare it for transition
    auto backbufferIndex = g_pSwapChain->GetCurrentBackBufferIndex();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(g_backBuffers[backbufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    frameCon->CommandList[m_frameResourceIndex]->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart(), m_frameResourceIndex, g_rtvDescriptorSize);
    frameCon->CommandList[m_frameResourceIndex]->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void gt::dx12::Draw(D3D12_VERTEX_BUFFER_VIEW& bufferView, uint64_t vertices, uint64_t instances)
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->IASetVertexBuffers(0, 1, &bufferView);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->DrawInstanced((UINT)vertices, (UINT)instances, 0, 0);
}

void gt::dx12::TansitionRtvToPresent()
{
    auto backbufferIndex = g_pSwapChain->GetCurrentBackBufferIndex();

    // Indicate that the back buffer will now be used to present.
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(g_backBuffers[backbufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->ResourceBarrier(1, &barrier2);
}

void gt::dx12::CloseCommandList()
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->Close();
}

void gt::dx12::MoveToNextFrame()
{
    // Get frame context and send to the command queu our fence value 
    auto frameCon = &m_frameContext[m_frameResourceIndex];
    ThrowIfFailed(m_pCommandQueue->Signal(m_fence.Get(), frameCon->FenceValue));

    // Update frame index
    //m_frameResourceIndex = g_pSwapChain->GetCurrentBackBufferIndex();
    m_frameResourceIndex = (m_frameResourceIndex + 1) % FRAME_COUNT;

    if (m_fence->GetCompletedValue() < frameCon->FenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(frameCon->FenceValue, m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    m_frameContext[m_frameResourceIndex].FenceValue = frameCon->FenceValue + 1;
}

void gt::dx12::CreateThreads()
{
    g_drawFinished = CreateEvent(NULL, FALSE, FALSE, L"Draw Finished");
    g_prepWorkDone = CreateEvent(NULL, FALSE, FALSE, L"Prep Work Done");
    // colored triangle
    auto work = [&](uint32_t threadIndex)
        {
            while (threadIndex < NUM_CONTEXT)
            {
                // Prepare allocators
                WaitForSingleObject(g_prepWorkDone, INFINITE);
                ThrowIfFailed(m_pCurrFrameContext->CommandAllocator[m_frameResourceIndex]->Reset());
                ThrowIfFailed(m_pCurrFrameContext->CommandList[m_frameResourceIndex]->Reset(m_pCurrFrameContext->CommandAllocator[m_frameResourceIndex].Get(), m_pPipelineState.Get()));
                /*for (auto i = 0u; i < Engine::FRAME_COUNT; ++i)
                {
                    ThrowIfFailed(m_pCurrFrameContext->CommandAllocator[i]->Reset());
                    ThrowIfFailed(m_pCurrFrameContext->CommandList[i]->Reset(m_pCurrFrameContext->CommandAllocator[i].Get(), m_pPipelineState.Get()));
                }*/
                SetViewport();
                ClearRTV(0.0f, 0.0f, 0.0f, 1.0f);

                m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetGraphicsRootSignature(m_pRootSignature.Get());
                m_pCurrFrameContext->CommandList[m_frameResourceIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                m_pCurrFrameContext->CommandList[m_frameResourceIndex]->IASetVertexBuffers(0, 1, &m_vertexBufferView);
                m_pCurrFrameContext->CommandList[m_frameResourceIndex]->DrawInstanced(3, 1, 0, 0);
                SetEvent(g_drawFinished);
            }
        };
    m_defaultDrawThread = std::jthread(work, 1);
    // Thread has synchronization events to wait for jobs when it is ready, let it run.
    // Though we will never fully close it correctly, which will be a problem later. Need to investigate stop tokens and 
    // see if I can kill it. Maybe jthread would be better to use? Async? More STL stuff?! 
    m_defaultDrawThread.detach();
    // textured triangle
    /*auto textureWork = [&](uint32_t threadIndex)
    {

    };*/

}

auto gt::dx12::DX12CubeApp::Initialize(int argC, char* argsV[]) -> LS::System::ErrorCode
{
    const auto& window = Window;
    if (!CreateDevice((HWND)window->GetHandleToWindow(), window->GetWidth(), window->GetHeight()))
        return LS::System::CreateFailCode("Failed to create device.");

    CreateCommandAllocators();
    CreateDescriptors();
    CreateSwapchain();
    CreateRenderTarget();
    //TODO: LoadContent is the Cube and LoadAssets is just the triangle 
    // but after loading we have to call the correct load to GPU function (loadContent doesn't have one yet)
    // still working on implementing some of the behaviors needed to set up everything
    if (LoadContent())
    {
        //LoadVertexDataToGpu();
    }
    
    // Create View Port //
    g_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
    // Scissor Rect is the actual drawing area of what will be rendered. A viewport can be bigger than the scissor rect,
    // or you can use Scissor rects to specify specific regions to draw (like omitting UI areas that may never be drawn because 2D render systems would handle that)
    g_scissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
    g_aspectRatio = g_viewport.Width / g_viewport.Height;
    m_fov = 45.0f;
    //m_cubeScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
    /*m_cubeViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight()));*/



    m_contentLoaded = false;
    return LS::System::CreateSuccessCode();
}

void gt::dx12::DX12CubeApp::Run()
{
    IsRunning = true;
    auto& window = Window;
    window->Show();
    /*while (window->IsOpen())
    {
        window->PollEvent();
        Render(1.0f, 1.0f, 0.0f, 1.0f);
    }*/
    const auto begin = std::chrono::high_resolution_clock::now();
    while (window->IsOpen())
    {
        const auto tick = std::chrono::high_resolution_clock::now();
        const auto end = std::chrono::high_resolution_clock::now();

        const auto delta = end - tick;
        const auto total = end - begin;
        float angle = static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(total).count() * 90.0f);
        const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
        m_cubeModelMat = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

        const XMVECTOR eyePos = XMVectorSet(0, 0, -10, 1);
        const XMVECTOR focus = XMVectorSet(0, 0, 0, 1);
        const XMVECTOR up  = XMVectorSet(0, 1, 0, 1);

        m_cubeViewMat = XMMatrixLookAtLH(eyePos, focus, up);

        float aspectRatio = window->GetWidth() / static_cast<float>(window->GetHeight());
        m_cubeProjMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), aspectRatio, 0.1f, 100.0f);

        //auto commandAllocator = 
    }
}

void gt::dx12::DX12CubeApp::UpdateBufferResource(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;
    // Create a committed resource for the GPU int he default heap
    const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto defaultBuffDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
    const auto hr = m_pDevice->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &defaultBuffDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(pDestinationResource));

    LS::Utils::ThrowIfFailed(hr, "Failed to commit resource onto the GPU");

    // Create the upload buffer 
    if (bufferData)
    {
        const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        const auto uploadHr = m_pDevice->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pIntermediateResource));

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
    auto commandList = m_copyCommandQueue->GetCommandList();

    // Upload vertex buffer data //
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

    // Create the Depth Stencil View
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = LS::Platform::Dx12::CreateDepthStencilViewDescriptor(1);

    ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    // Find current directory to get our path and add the shaders (they'll be local to the exe for now)
    std::array<wchar_t, _MAX_PATH> modulePath{};
    if (!GetModuleFileName(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size())))
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
        { .SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 },
        { .SemanticName = "COLOR", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, .InstanceDataStepRate = 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
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
    ThrowIfFailed(m_pDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
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
    ThrowIfFailed(m_pDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_cubePipelineState)));

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
    auto fenceValue = m_copyCommandQueue->ExecuteCommandList(commandList);
    m_copyCommandQueue->WaitForFenceValue(fenceValue);

    m_contentLoaded = true;
    // Resize/Create the depth buffer.
    const auto& window = Window;
    ResizeDepthBuffer(window->GetWidth(), window->GetHeight());

    return true;
}

void gt::dx12::DX12CubeApp::ResizeDepthBuffer(int width, int height)
{
    if (m_contentLoaded)
    {
        m_copyCommandQueue->Flush();
        m_directCommandQueue->Flush();
        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };
        const CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
        const auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        ThrowIfFailed(m_pDevice->CreateCommittedResource(
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

        m_pDevice->CreateDepthStencilView(m_depthBuffer.Get(), &dsv,
            m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void gt::dx12::DX12CubeApp::TransitionResource(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), before, after);
    commandList->ResourceBarrier(1, &barrier);
}

void gt::dx12::DX12CubeApp::ClearRTV(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, float* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void gt::dx12::DX12CubeApp::ClearDepth(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, float depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void gt::dx12::Render(float r, float g, float b, float a)
{
    if (firstRun)
    {
        CreateThreads();
        firstRun = false;
    }

    BeginRender();
    // Notify that we did initial work required before render systems can start
    SetEvent(g_prepWorkDone);
    //m_defaultDrawThread.detach();
    // Wait for render system to finish (currently only one)
    WaitForSingleObject(g_drawFinished, INFINITE);
    // In the D3D12Multithreading example, it looks like they have separate command lists for pre/mid/post
    // rendering work.This method could be considered, but how they achieve that is also through having a single batch command queue
    // where the order of each queue is split between pre work, which is first command list, then some N amount of mid work, followed by the post work.
    // We could learn to take a similar approach. Right now each FrameContext has 3 command allocators (current setting) and command lists.
    // We could make 0 = prep work, 1 = mid (render), 2 = finalization. 
    TansitionRtvToPresent();
    // This probably could be handled by the actual thread, before it signals it is done
    // because it belongs to the current command list, and this is out of scope. 
    // However PresentRTV handles the transition, so need to examine how to handle that first
    // before we can close. 
    CloseCommandList();
    // Throw command list onto the command queue and prepare to send it off
    ExecuteCommandList();
    ThrowIfFailed(g_pSwapChain->Present(1, 0));

    g_frameIndex = g_pSwapChain->GetCurrentBackBufferIndex();
    // Wait for next frame
    MoveToNextFrame();
}

void gt::dx12::OnDestroy()
{
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}