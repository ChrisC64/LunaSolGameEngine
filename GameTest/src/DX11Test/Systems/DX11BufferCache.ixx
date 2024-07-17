module;
#include <wrl/client.h>
#include <d3d11_4.h>

export module DX11Systems:DX11BufferCache;
import <span>;
import <unordered_map>;
import <string>;
import <string_view>;
import <cstdint>;
import <format>;
import <optional>;
import <filesystem>;

import Engine.EngineCodes;
import Engine.Defines;
import Win32.ComUtils;
import D3D11.MemoryHelper;
import Helper.IO;

/*
* A class that isn't really much. I will over time try different strategies. 
* I don't like using strings to hold names and references as much, and
* maybe I pass back IDs (hashed from the initial string name or just a hash of the object?)
* I also don't really have a clue of how I want to lock things. I could just use mutex
* or maybe I use some atomic value or even a latch/barrier type scenario.
* 
* As for now, nothing I use is currently multi-threaded, so it may be this will change
* drastically, which is why I am not committing anything to the library yet (maybe I never
* will?)
*/

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
namespace LS::Platform::Dx11
{
    enum class LockState
    {
        LOCKED,
        UNLOCKED,
    };

    struct BufferContents
    {
        ComPtr<ID3D11Buffer> Buffer;
        LockState State = LockState::UNLOCKED;
    };
}

export namespace LS::Platform::Dx11
{
    using Key = uint32_t;
    using Values = BufferContents;
    using Cache = std::unordered_map<Key, Values>;
    template<class T>
    using ResMap = std::unordered_map<Key, T>;

    template<class T>
    class ShaderMap
    {
    public:
        ShaderMap() = default;
        ~ShaderMap() = default;

        ShaderMap(const ShaderMap&) = delete;
        ShaderMap(ShaderMap&&) = default;

        ShaderMap& operator=(const ShaderMap&) = delete;
        ShaderMap& operator=(ShaderMap&&) = default;

    private:
        Key m_key = 0;
        ResMap<T> m_map;

    public:
        [[nodiscard]]
        auto Add(const T& obj) noexcept -> Nullable<Key>
        {
            const auto [_, status] = m_map.emplace(m_key, obj);

            if (!status)
                return std::nullopt;
            m_key++;
            return m_key - 1;
        }

        [[nodiscard]]
        auto Get(Key key) noexcept -> Nullable<T>
        {
            if (!m_map.contains(key))
                return std::nullopt;

            return m_map.at(key);
        }
    };


    class BufferCache
    {
    public:
        BufferCache() = default;
        ~BufferCache() = default;

        BufferCache(const BufferCache&) = delete;
        BufferCache& operator=(const BufferCache&) = delete;

        BufferCache(BufferCache&&) = default;
        BufferCache& operator=(BufferCache&&) = default;

    private:
        /**
         * @brief Insert a buffer into the cache
         * @param key a unique ID
         * @param buffer The buffer to insert
         * @return A success error code means insertion took place, a fail error code means insertion did not
        */
        [[nodiscard]]
        auto Insert(ComPtr<ID3D11Buffer> buffer) noexcept -> Nullable<Key>
        {
            if (!buffer)
                return std::nullopt;

            Utils::SetDebugName(buffer.Get(), "Buffer_" + std::to_string(m_key));
            BufferContents bc{ .Buffer = buffer };
            const auto [_, status] = m_cache.emplace(m_key, bc);

            if (!status)
                return std::nullopt;
            m_key++;
            return m_key - 1;
        }

    public:
        [[nodiscard]]
        auto Get(Key key) noexcept -> Nullable<ComPtr<ID3D11Buffer>>
        {
            if (!m_cache.contains(key) || m_cache.at(key).State == LockState::LOCKED)
                return std::nullopt;

            return m_cache.at(key).Buffer;
        }

        [[nodiscard]]
        auto Remove(Key key) noexcept -> LS::System::ErrorCode
        {
            if (!m_cache.contains(key))
            {
                return LS::System::CreateFailCode(std::format("Cannot remove buffer: {} It does not exist.", key));
            }

            if (m_cache.at(key).State == LockState::LOCKED)
            {
                return LS::System::CreateFailCode(std::format("Buffer {} is currently locked.", key));
            }
            m_cache.erase(key);
            return LS::System::CreateSuccessCode();
        }

        [[nodiscard]]
        auto Lock(Key key) noexcept -> Nullable<ComPtr<ID3D11Buffer>>
        {
            if (!m_cache.contains(key) || m_cache.at(key).State == LockState::LOCKED)
                return std::nullopt;

            return m_cache.at(key).Buffer;
        }

        void Unlock(Key key) noexcept
        {
            if (m_cache.contains(key) && m_cache.at(key).State == LockState::LOCKED)
            {
                m_cache.at(key).State = LockState::UNLOCKED;
                return;
            }
        }

        void SetCacheSize(std::uint64_t size) noexcept
        {
            m_limit = size;
        }

        [[nodiscard]]
        auto GetCacheSize() noexcept -> std::uint64_t
        {
            return m_limit;
        }

        [[nodiscard]]
        auto CreateIndexBuffer(std::span<uint32_t> indices, ID3D11Device* pDevice) noexcept -> Nullable<Key>
        {
            const auto ibOpt = LS::Platform::Dx11::CreateIndexBuffer(pDevice, indices);
            if (!ibOpt)
            {
                return std::nullopt;
            }
            
            return Insert(ibOpt.value());
        }

        /*template<class T>
        [[nodiscard]]
        auto CreateVertexBuffer(const T& data, ID3D11Device* pDevice) noexcept -> Nullable<Key>
        {
            const auto ibOpt = LS::Platform::Dx11::CreateVertexBuffer(pDevice, data);
            if (!ibOpt)
            {
                return std::nullopt;
            }
            
            return Insert(ibOpt.value());
        }*/

        template<class T>
        [[nodiscard]]
        auto CreateConstantBuffer(const T* data, size_t size, ID3D11Device* pDevice) noexcept -> Nullable<Key>
        {
            const auto ibOpt = LS::Platform::Dx11::CreateConstantBuffer(pDevice, data, size);
            if (!ibOpt)
            {
                return std::nullopt;
            }
            
            return Insert(ibOpt.value());
        }
        
        template<class T>
        [[nodiscard]]
        auto CreateVertexBuffer(const T* data, size_t size, ID3D11Device* pDevice) noexcept -> Nullable<Key>
        {
            const auto ibOpt = LS::Platform::Dx11::CreateVertexBuffer(pDevice, data, size);
            if (!ibOpt)
            {
                return std::nullopt;
            }
            
            return Insert(ibOpt.value());
        }

    private:
        std::uint64_t m_limit = 1024u * 1024u * 1024u * 3;
        Cache m_cache;
        Key m_key = 0;
    };
}