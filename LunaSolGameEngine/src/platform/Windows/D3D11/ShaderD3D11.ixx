module;
#include <vector>
#include <string>
#include <wrl/client.h>
#include <d3d11_4.h>
export module D3D11.ShaderD3D11;

import Engine.Defines;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    template <class T>
    struct ShaderResource
    {
        uint32_t Slot;
        T Resource;
        std::string Name;

        constexpr bool operator==(uint32_t slot) const
        {
            return Slot == slot;
        }

        constexpr bool operator==(const char* c) const
        {
            return Name == c;
        }
    };

    template <class T>
    using ShaderResArray = std::vector<ShaderResource<T>>;

    template <class T>
    class ShaderD3D11
    {
    public:
        ShaderD3D11() = default;
        ~ShaderD3D11() = default;

        auto GetShader() const noexcept
        {
            return m_shader;
        }

        auto GetConstantBuffer(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>;
        auto GetConstantBuffer(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>;
        auto GetTexture(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>;
        auto GetTexture(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>;
        auto GetSampler(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>;
        auto GetSampler(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>;
        auto GetUav(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>;
        auto GetUav(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>;

    private:
        T m_shader;
        ShaderResArray<WRL::ComPtr<ID3D11Buffer>> m_constantBuffers;
        ShaderResArray<WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
        ShaderResArray<WRL::ComPtr<ID3D11SamplerState>> m_samplers;
        ShaderResArray<WRL::ComPtr<ID3D11UnorderedAccessView>> m_uavs;
    };

    template<class T>
    auto ShaderD3D11<T>::GetConstantBuffer(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11Buffer>>(m_constantBuffers, id);
    }

    template<class T>
    auto ShaderD3D11<T>::GetConstantBuffer(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11Buffer>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11Buffer>>(m_constantBuffers, slot);
    }

    template<class T>
    auto ShaderD3D11<T>::GetTexture(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11ShaderResourceView>>(m_textures, id);
    }

    template<class T>
    auto ShaderD3D11<T>::GetTexture(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11ShaderResourceView>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11ShaderResourceView>>(m_textures, slot);
    }

    template<class T>
    auto ShaderD3D11<T>::GetSampler(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11SamplerState>>(m_samplers, id);
    }

    template<class T>
    auto ShaderD3D11<T>::GetSampler(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11SamplerState>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11SamplerState>>(m_samplers, slot);
    }

    template<class T>
    auto ShaderD3D11<T>::GetUav(const char* id) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11UnorderedAccessView>>(m_uavs, id);
    }

    template<class T>
    auto ShaderD3D11<T>::GetUav(uint32_t slot) const noexcept -> Nullable<WRL::ComPtr<ID3D11UnorderedAccessView>>
    {
        return FindOrNull<WRL::ComPtr<ID3D11UnorderedAccessView>>(m_uavs, slot);
    }
}