module;
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <vector>
#include <cassert>
export module DXGIHelper;
export import Data.LSDataTypes;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    /**
     * @brief Creates a DXGI Factory
     * @param flags DXGI_CREATE_FLAGS
     * @return @link Nullable object of a IDXGIFactory7 instance (see module @link LSdataTypes.ixx)
    */
    [[nodiscard]] auto CreateFactory(UINT flags = 0u) noexcept -> Nullable<IDXGIFactory7*>;

    /**
     * @brief Returns any high performance display adapters (discrete or external GPU supported)
     * @param pFactory The IDXGIFactory, must be convertable to a IDXGIFactory6 object
     * @return A vector of all Discrete GPU supported displays if any are found.
    */
    [[nodiscard]] auto EnumerateDiscreteGpuAdapters(IDXGIFactory1* pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>;

    /**
     * @brief Returns any display adapters
     * @param pFactory The IDXGIFactory interface object to use
     * @return A container containing all display adapters
    */
    [[nodiscard]] auto EnumerateDisplayAdapters(IDXGIFactory1* pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>;
}


module : private;

using namespace LS::Win32;

auto CreateFactory(UINT flags) noexcept -> Nullable<IDXGIFactory7*>
{
    IDXGIFactory7* pOut;
    auto result = CreateDXGIFactory2(flags, IID_PPV_ARGS(&pOut));
    if (FAILED(result))
        return {};
    return pOut;
}

auto EnumerateDiscreteGpuAdapters(IDXGIFactory1* pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>
{
    assert(pFactory);
    WRL::ComPtr<IDXGIAdapter4> adapter;
    WRL::ComPtr<IDXGIFactory6> factory6;

    auto result = SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)));

    std::vector<WRL::ComPtr<IDXGIAdapter4>> out;
    if (FAILED(result))
    {
        // Unable to find the high performance adapter because we cannot obtain the proper interface
        return out;
    }

    auto preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
    for (auto adapterIndex = 0u; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, preference, IID_PPV_ARGS(&adapter)));
        ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            //TODO: Maybe add optional support for software adapter types 
            continue;
        }
        out.push_back(std::move(adapter));
    }

    return out;
}

auto EnumerateDisplayAdapters(IDXGIFactory1* pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>
{
    assert(pFactory);
    std::vector<WRL::ComPtr<IDXGIAdapter4>> out;

    WRL::ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            //TODO: Maybe add optional support for software adapter types 
            continue;
        }
        WRL::ComPtr<IDXGIAdapter4> move;
        if (FAILED(adapter.As(&move)))
        {
            break;
        }
        out.push_back(std::move(move));
    }

    return out;
}