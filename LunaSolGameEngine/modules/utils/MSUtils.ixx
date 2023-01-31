module;
#include "LSEFramework.h"
export module Util.MSUtils;

//Utility functions for Microsoft specific stuff //
export namespace LS::Utils
{
#ifdef LS_WINDOWS_BUILD
    template <typename TType>
    inline constexpr void ComRelease(TType** comPtr)
    {
        if (*comPtr)
        {
            (*comPtr)->Release();
            *comPtr = NULL;
        }
    }
#endif LS_WINDOWS_BUILD

    constexpr void SetDebugName([[maybe_unused]] ID3D11DeviceChild* child, [[maybe_unused]] const std::string& name)
    {
#ifdef _DEBUG
        if (child)
        {
            child->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
        }
#endif _DEBUG
    }

    constexpr void SetDebugName([[maybe_unused]] ID3D11Device* device, [[maybe_unused]] const std::string& name)
    {
#ifdef _DEBUG
        if (device)
        {
            device->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
        }
#endif _DEBUG
    }

    constexpr void SetDebugName([[maybe_unused]] IDXGIObject* object, [[maybe_unused]] const std::string& name)
    {
#ifdef _DEBUG
        if (object)
        {
            object->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
        }
#endif _DEBUG
    }

    constexpr void ThrowIfFailed(HRESULT hr, const char* s)
    {
        if (FAILED(hr))
        {
            throw std::runtime_error(s);
        }
    }

    template<class T, class U>
    constexpr HRESULT QueryInterfaceFor(Microsoft::WRL::ComPtr<T>& obj, Microsoft::WRL::ComPtr<U>& query)
    {
        return obj->QueryInterface(IID_PPV_ARGS(&query));
    }
    
    template<class T, class U>
    constexpr HRESULT QueryInterfaceFor(T* obj, U** query)
    {
        return obj->QueryInterface(IID_PPV_ARGS(query));
    }
    
    // DXGI Devices Queries //

    /**
     * @brief Creates a DXGIDevice4 object if it HRESULT SUCCESS or sets it to nullptr if it FAILS
     * @param obj The device object to use for the query
     * @param dxgiDevice the object to instantiate if HRESULT returns SUCCESS or nullptr if FAILED
     * @return initializes the DXGIDevice object if SUCCESS or nullptr if FAILED
    */
    constexpr HRESULT QueryDXGIDevice4(ID3D11Device5* obj, IDXGIDevice4** dxgiDevice)
    {
        HRESULT hr = QueryInterfaceFor(obj, dxgiDevice);
        if (FAILED(hr))
            dxgiDevice = nullptr;
        return hr;
    }

    // DXGI Adapter Queries //

    /**
     * @brief Creates a IDXGIAdapter object if it HRESULT SUCCESS or sets it to nullptr if it FAILS
     * @param obj The device object to use for the query
     * @param dxgiAdapter the object to instantiate if HRESULT returns SUCCESS or nullptr if FAILED
     * @return initializes the IDXGIAdapter object if SUCCESS or nullptr if FAILED
    */
    constexpr HRESULT QueryDXGIAdapter(IDXGIDevice1* obj, IDXGIAdapter** dxgiAdapter)
    {
        HRESULT hr = obj->GetAdapter(dxgiAdapter);
        if (FAILED(hr))
            dxgiAdapter = nullptr;
        return hr;
    }

    // DXGI Factory Queries //

    /**
     * @brief Creates a DXGIFactory2 object if it HRESULT SUCCESS or sets it to nullptr if it FAILS
     * @param obj The device object to use for the query
     * @param dxgiFactory the object to instantiate if HRESULT returns SUCCESS or nullptr if FAILED
     * @return initializes the DXGIFactory2 object if SUCCESS or nullptr if FAILED
    */
    constexpr HRESULT QueryDXGIFactory2(IDXGIAdapter* obj, IDXGIFactory2** dxgiFactory)
    {
        HRESULT hr = obj->GetParent(IID_PPV_ARGS(dxgiFactory));
        if (FAILED(hr))
            dxgiFactory = nullptr;
        return hr;
    }

}