module;

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

export module CubeApp;

import LSData;
import Engine.Common;
import D3D11Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Helper.PipelineFactory;


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

namespace gt
{
    export LS::System::ErrorCode Init();
    export void Run();

    namespace WRL = Microsoft::WRL;

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

    struct ThreadParameter
    {
        uint32_t threadIndex;
    };
    ThreadParameter m_threadParameters[NUM_CONTEXT];

    std::array<FrameContext, FRAME_COUNT>					m_frameContext = {};
    FrameContext* m_pCurrFrameContext;
    uint32_t												m_frameResourceIndex;
    uint32_t												m_frameIndex;
    float													m_aspectRatio;

    // pipeline objects
    WRL::ComPtr<ID3D12Device4>								m_pDevice;
    WRL::ComPtr<ID3D12DescriptorHeap>						m_pRtvDescHeap;
    WRL::ComPtr<ID3D12DescriptorHeap>						m_pSrvDescHeap;
    WRL::ComPtr<ID3D12CommandAllocator>						m_pCommandAllocator; // local command allocator for non-frame resource related jobs.
    WRL::ComPtr<ID3D12CommandAllocator>						m_pCommandAllocator2; // local command allocator for non-frame resource related jobs.
    WRL::ComPtr<ID3D12CommandQueue>							m_pCommandQueue;
    WRL::ComPtr<IDXGISwapChain4>							m_pSwapChain = nullptr;
    //ComPtr<ID3D12GraphicsCommandList>						m_pCommandList; // Records drawing or state chaning calls for execution later by the GPU - Set states, draw calls - think the D3D11::ImmediateContext 
    WRL::ComPtr<ID3D12CommandAllocator>						m_pBundleAllocator;
    WRL::ComPtr<ID3D12GraphicsCommandList>					m_pBundleList;
    WRL::ComPtr<ID3D12RootSignature>						m_pRootSignature; // Used with shaders to determine input and variables
    WRL::ComPtr<ID3D12RootSignature>						m_pRootSignature2; // Used with shaders to determine input and variables - texture_effect.hlsl
    WRL::ComPtr<ID3D12PipelineState>						m_pPipelineState; // Defines our pipeline's state - primitive topology, render targets, shaders, etc. 
    WRL::ComPtr<ID3D12PipelineState>						m_pPipelineStatePT; // Defines our pipeline's state - primitive topology, render targets, shaders, etc. 
    HANDLE													m_hSwapChainWaitableObject = nullptr;
    std::array<WRL::ComPtr<ID3D12Resource>, FRAME_COUNT>	m_mainRenderTargetResource = {};// Our Render Target resources
    D3D12_CPU_DESCRIPTOR_HANDLE								m_mainRenderTargetDescriptor[FRAME_COUNT] = {};
    CD3DX12_VIEWPORT										m_viewport;
    CD3DX12_RECT											m_scissorRect;

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
    UINT													m_rtvDescriptorSize = 0;
    HANDLE													m_drawFinished;
    HANDLE													m_prepWorkDone;
    HANDLE													m_beginRender;
    //std::array<std::thread, Engine::NUM_CONTEXT>			m_threadPool;
    std::jthread											m_defaultDrawThread;
    std::jthread											m_textureDrawThread;
    // Blah stuff
    bool firstRun = true;

    bool CreateDevice(HWND hwnd, uint32_t x, uint32_t y);
    bool LoadAssets();
    void LoadVertexDataToGpu();
    void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter);
    void CreateRenderTarget();
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
    void SetDescriptorHeaps();
    void SetViewport();
    void ClearRTV(float r, float g, float b, float a);
    void SetRTV(FrameContext* frameCon);
    void Draw(D3D12_VERTEX_BUFFER_VIEW& bufferView, uint64_t vertices, uint64_t instances = 1u);
    void PresentRTV();
    void CloseCommandList();
    void MoveToNextFrame();
    void CreateThreads();
    void Render(float r, float g, float b, float a);
    void OnDestroy();

    export auto App = LS::CreateAppRef(800, 600, L"Cube App", std::move(Init), std::move(Run));

    using namespace LS;
    export LS::System::ErrorCode Init()
    {
        if (!CreateDevice((HWND)App->Window->GetHandleToWindow(), App->Window->GetWidth(), App->Window->GetHeight()))
            return System::FailErrorCode(System::ErrorCategory::GENERAL, "Failed to create device.");

        return System::SuccessErrorCode();
    }
    export void Run()
    {
        auto& window = App->Window;
        window->Show();
        while (window->IsOpen())
        {
            window->PollEvent();
            Render(1.0f, 1.0f, 0.0f, 1.0f);

        }
    }

    inline WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        uint64_t byteSize,
        WRL::ComPtr<ID3D12Resource>& uploadBuffer,
        D3D12_RESOURCE_STATES finalState,
        std::optional<std::wstring_view> defaultName = std::nullopt,
        std::optional<std::wstring_view> uploadName = std::nullopt)
    {
        WRL::ComPtr<ID3D12Resource> defaultBuffer;

        // Creat Default Buffer
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

    inline WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer2(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        uint64_t byteSize,
        WRL::ComPtr<ID3D12Resource>& uploadBuffer,
        D3D12_RESOURCE_STATES finalState)
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

    std::vector<UINT8> GenerateTextureData(uint32_t textureWidth, uint32_t textureHeight, uint32_t pixelSize)
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

    void CreateTileSampleTexture(ID3D12Device* device, WRL::ComPtr<ID3D12Resource>& texture, uint32_t textureWidth,
        uint32_t textureHeight, uint32_t pixelSize, WRL::ComPtr<ID3D12Resource>& textureUploadHeap,
        WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, WRL::ComPtr<ID3D12DescriptorHeap>& srvHeap)
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
}

module : private;
using namespace gt;

bool gt::CreateDevice(HWND hwnd, uint32_t x, uint32_t y)
{
    // [DEBUG] Enable debug interface
#ifdef _DEBUG
    WRL::ComPtr<ID3D12Debug> pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif
    // Create our DXGI Factory
    WRL::ComPtr<IDXGIFactory7> factory;
    CreateDXGIFactory2(0u, IID_PPV_ARGS(&factory));

    // Find the best graphics card (best performing one, with single GPU systems, this should be the default)
    WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(factory.Get(), &hardwareAdapter, false);

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
    // Create command queue - This is a FIFO structure used to send commands to the GPU
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;

        if (m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue)) != S_OK)
            return false;
    }

    bool useRect = x == 0 || y == 0;
    long width{}, height{};
    RECT rect;
    if (useRect && GetWindowRect(hwnd, &rect))
    {
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = FRAME_COUNT;
    swapchainDesc1.Width = useRect ? static_cast<UINT>(width) : x;
    swapchainDesc1.Height = useRect ? static_cast<UINT>(height) : y;
    swapchainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    swapchainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc1.SampleDesc.Count = 1;
    swapchainDesc1.SampleDesc.Quality = 0;
    swapchainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc1.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc1.Stereo = FALSE;

    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(swapchainDesc1.Width), static_cast<float>(swapchainDesc1.Height));
    // Scissor Rect is the actual drawing area of what will be rendered. A viewport can be bigger than the scissor rect,
    // or you can use Scissor rects to specify specific regions to draw (like omitting UI areas that may never be drawn because 2D render systems would handle that)
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(swapchainDesc1.Width), static_cast<LONG>(swapchainDesc1.Height));
    m_aspectRatio = m_viewport.Width / m_viewport.Height;
    // Since we are using an HWND (Win32) system, we can create the swapchain for HWND 
    {
        WRL::ComPtr<IDXGISwapChain1> swapChain1 = nullptr;
        if (factory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hwnd, &swapchainDesc1, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1.As(&m_pSwapChain) != S_OK)
            return false;

        // Helper function that displays our display's resolution and refresh rates and other information 
        LogAdapters(factory.Get());
        // Don't allot ALT+ENTER fullscreen
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        m_pSwapChain->SetMaximumFrameLatency(FRAME_COUNT);
        m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        m_hSwapChainWaitableObject = m_pSwapChain->GetFrameLatencyWaitableObject();
    }

    // Descriptor - a block of data that describes an object to the GPU (SRV, UAVs, CBVs, RTVs, DSVs)
    // Descriptor Heap - A collection of contiguous allocations of descriptors
    // This is the RTV descriptor heap (render target view)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NumDescriptors = FRAME_COUNT;
        //desc.NodeMask = 1;

        if (m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pRtvDescHeap)) != S_OK)
            return false;
        // Handles have a size that varies by GPU, so we have to ask for the Handle size on the GPU before processing
        m_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // a descriptor heap for the Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NumDescriptors = 1;

        if (m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pSrvDescHeap)) != S_OK)
            return false;
    }

    CreateRenderTarget();

    bool isSuccess = LoadAssets();
    if (isSuccess)
    {
        LoadVertexDataToGpu();
    }

    return isSuccess;
}

bool gt::LoadAssets()
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

    m_pCurrFrameContext = &m_frameContext[m_frameIndex];
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

void gt::LoadVertexDataToGpu()
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
        m_vertexBuffer = CreateDefaultBuffer(m_pDevice.Get(), commandList.Get(), triangleVertices, sizeof(Vertex) * 3, uploadBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, L"default vb");

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
            { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 0.5f, 0.0f } },
            { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 1.0f, 1.0f } },
            { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f } }
        };
        const UINT vertexBufferSize2 = sizeof(triangleVerticesPT);

        WRL::ComPtr<ID3D12Resource> textureUploadBuffer;
        m_vertexBufferPT = CreateDefaultBuffer(m_pDevice.Get(), commandList2.Get(), triangleVerticesPT, sizeof(VertexPT) * 3, textureUploadBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, L"pt default vb");
        WRL::ComPtr<ID3D12Resource> textureUploadHeap;
        {
            CreateTileSampleTexture(m_pDevice.Get(), m_texture, 256u, 256u, 4u, textureUploadHeap, commandList2, m_pSrvDescHeap);
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

void gt::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
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

void gt::CreateRenderTarget()
{
    // The handle can now be used to help use build our RTVs - one RTV per frame/back buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < FRAME_COUNT; i++)
    {
        ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_mainRenderTargetResource[i])));
        m_pDevice->CreateRenderTargetView(m_mainRenderTargetResource[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
        m_mainRenderTargetDescriptor[i] = rtvHandle;
    }
    ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)));
    m_pCommandAllocator->SetName(L"Data Command Allocator");
    ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator2)));
    m_pCommandAllocator2->SetName(L"Data Command Allocator 2");
}

void gt::CheckFeatures([[maybe_unused]] std::string& s)
{
}

void gt::LogAdapters(IDXGIFactory4* factory)
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

void gt::LogAdapterOutput(IDXGIAdapter* adapter)
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

void gt::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
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

void gt::ExecuteCommandList()
{
    // Execut the command list
    /*ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);*/
    ID3D12CommandList* ppCommandLists[] = { m_pCurrFrameContext->CommandList[m_frameResourceIndex].Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

// Waits for work on the GPU to finish before moving on to the next frame
void gt::WaitForGpu()
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

//void Render(const ColorRGBA& clearColor)
//{
//	// Reset command allocator to claim memory used by it
//	// Then reset the command list to its default state
//	// Perform commands for the new state (just clear screen in this example)
//	// Uses a resource barrier to manage transition of resource (Render Target) from one state to another
//	// Close command list to execute the command

//	BeginRender();
//	// Basic setup for drawing - Reset command list, set viewport to draw to, and clear the frame buffer
//	ResetCommandList();
//	SetViewport();
//	ClearRTV(clearColor);
//	// Draws the gradient triangle
//	SetPipelineState(m_pPipelineState);
//	m_pCurrFrameContext->CommandList[m_frameResourceIndex]->ExecuteBundle(m_pBundleList.Get());

//	// set the state of the pipeline for the textured triangle
//	SetPipelineState(m_pPipelineStatePT);
//	SetRootSignature(m_pRootSignature2);
//	SetDescriptorHeaps();
//	Draw(m_vertexBufferViewPT, 3);
//	// Prepare to render to the render target
//	PresentRTV();
//	CloseCommandList();
//	// Throw command list onto the command queue and prepare to send it off
//	ExecuteCommandList();
//	ThrowIfFailed(m_pSwapChain->Present(1, 0));

//	m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
//	// Wait for next frame
//	MoveToNextFrame();
//}

void gt::BeginRender()
{
    m_pCurrFrameContext = &m_frameContext[m_frameResourceIndex];
    // Reclaims the memory allocated by this allocator for our next usage
    ThrowIfFailed(m_pCurrFrameContext->CommandAllocator[m_frameResourceIndex]->Reset());
}

void gt::ResetCommandList()
{
    // Resets a command list to its initial state 
    ThrowIfFailed(m_pCurrFrameContext->CommandList[m_frameResourceIndex]->Reset(m_pCurrFrameContext->CommandAllocator[m_frameResourceIndex].Get(), nullptr));
}

void gt::SetPipelineState(WRL::ComPtr<ID3D12PipelineState>& pipelineState)
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetPipelineState(pipelineState.Get());
}

void gt::SetRootSignature(WRL::ComPtr<ID3D12RootSignature>& rootSignature)
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetGraphicsRootSignature(rootSignature.Get());
}

void gt::SetDescriptorHeaps()
{
    ID3D12DescriptorHeap* ppHeaps[] = { m_pSrvDescHeap.Get() };
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->SetGraphicsRootDescriptorTable(0, m_pSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
}

void gt::SetViewport()
{
    // Set Viewport
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->RSSetViewports(1, &m_viewport);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->RSSetScissorRects(1, &m_scissorRect);
}

void gt::ClearRTV(float r, float g, float b, float a)
{
    // This will prep the back buffer as our render target and prepare it for transition
    auto backbufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_mainRenderTargetResource[backbufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart(), m_frameResourceIndex, m_rtvDescriptorSize);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    // Similar to D3D11 - this is our command for drawing. For now, testing triangle drawing through MSDN example code
    const float color[] = { r, g, b, a };
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

}

void gt::SetRTV(FrameContext* frameCon)
{
    // This will prep the back buffer as our render target and prepare it for transition
    auto backbufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_mainRenderTargetResource[backbufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    frameCon->CommandList[m_frameResourceIndex]->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart(), m_frameResourceIndex, m_rtvDescriptorSize);
    frameCon->CommandList[m_frameResourceIndex]->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void gt::Draw(D3D12_VERTEX_BUFFER_VIEW& bufferView, uint64_t vertices, uint64_t instances)
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->IASetVertexBuffers(0, 1, &bufferView);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->DrawInstanced(vertices, instances, 0, 0);
}

void gt::PresentRTV()
{
    auto backbufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    // Indicate that the back buffer will now be used to present.
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_mainRenderTargetResource[backbufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->ResourceBarrier(1, &barrier2);
}

void gt::CloseCommandList()
{
    m_pCurrFrameContext->CommandList[m_frameResourceIndex]->Close();
}

void gt::MoveToNextFrame()
{
    // Get frame context and send to the command queu our fence value 
    auto frameCon = &m_frameContext[m_frameResourceIndex];
    ThrowIfFailed(m_pCommandQueue->Signal(m_fence.Get(), frameCon->FenceValue));

    // Update frame index
    //m_frameResourceIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    m_frameResourceIndex = (m_frameResourceIndex + 1) % FRAME_COUNT;

    if (m_fence->GetCompletedValue() < frameCon->FenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(frameCon->FenceValue, m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    m_frameContext[m_frameResourceIndex].FenceValue = frameCon->FenceValue + 1;
}

void gt::CreateThreads()
{
    m_drawFinished = CreateEvent(NULL, FALSE, FALSE, L"Draw Finished");
    m_prepWorkDone = CreateEvent(NULL, FALSE, FALSE, L"Prep Work Done");
    // colored triangle
    auto work = [&](uint32_t threadIndex)
    {
        while (threadIndex < NUM_CONTEXT)
        {
            // Prepare allocators
            WaitForSingleObject(m_prepWorkDone, INFINITE);
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
            SetEvent(m_drawFinished);
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

void gt::Render(float r, float g, float b, float a)
{
    if (firstRun)
    {
        CreateThreads();
        firstRun = false;
    }

    BeginRender();
    // Notify that we did initial work required before render systems can start
    SetEvent(m_prepWorkDone);
    //m_defaultDrawThread.detach();
    // Wait for render system to finish (currently only one)
    WaitForSingleObject(m_drawFinished, INFINITE);
    // In the D3D12Multithreading example, it looks like they have separate command lists for pre/mid/post
    // rendering work.This method could be considered, but how they achieve that is also through having a single batch command queue
    // where the order of each queue is split between pre work, which is first command list, then some N amount of mid work, followed by the post work.
    // We could learn to take a similar approach. Right now each FrameContext has 3 command allocators (current setting) and command lists.
    // We could make 0 = prep work, 1 = mid (render), 2 = finalization. 
    PresentRTV();
    // This probably could be handled by the actual thread, before it signals it is done
    // because it belongs to the current command list, and this is out of scope. 
    // However PresentRTV handles the transition, so need to examine how to handle that first
    // before we can close. 
    CloseCommandList();
    // Throw command list onto the command queue and prepare to send it off
    ExecuteCommandList();
    ThrowIfFailed(m_pSwapChain->Present(1, 0));

    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    // Wait for next frame
    MoveToNextFrame();
}

void gt::OnDestroy()
{
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}