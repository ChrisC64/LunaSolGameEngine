module;
#include <wrl/client.h>
#include <d3d11_4.h>
#include <unordered_map>
#include <string>
#include <string_view>
#include <cstdint>
#include <format>
#include <optional>
#include "engine/EngineDefines.h"
export module DX11Systems:DX11BufferCache;
import Engine.EngineCodes;
import Util.MSUtils;

export namespace LS::Platform::Dx11
{
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
    using Buffer = ComPtr<ID3D11Buffer>;
    using BufferPtr = ID3D11Buffer*;
    using Cache = std::unordered_map<std::string, Buffer>;
    using CacheSize = std::uint64_t;

    class BufferCache
    {
    public:
        BufferCache() = default;
        ~BufferCache() = default;

        BufferCache(const BufferCache&) = delete;
        BufferCache& operator=(const BufferCache&) = delete;

        BufferCache(BufferCache&&) = default;
        BufferCache& operator=(BufferCache&&) = default;

        /**
         * @brief Insert a buffer into the cache
         * @param key a unique ID 
         * @param buffer The buffer to insert
         * @return A success error code means insertion took place, a fail error code means insertion did not
        */
        [[nodiscard]]
        auto Insert(std::string_view key, Buffer& buffer) noexcept -> LS::System::ErrorCode
        {
            Utils::SetDebugName(buffer.Get(), key);
            auto [_, status] = m_cache.emplace(key.data(), buffer);

            if (!status)
                return LS::System::CreateFailCode(std::format("Could not add key: {}", key));
            return LS::System::CreateSuccessCode();
        }
        
        /**
         * @brief Takes ownership of the ID3D11Buffer object and adds it to the cache. If not successful, will
         * return ownership back to the object
         * @param key A unique ID for the buffer
         * @param pBuffer The ID3D11Buffer* object to inser
         * @return A success error code means insertion took place, a fail error code means it did not get added
        */
        [[nodiscard]]
        auto Insert(std::string_view key, BufferPtr pBuffer) noexcept -> LS::System::ErrorCode
        {
            Buffer buffer;
            buffer.Attach(pBuffer);
            Utils::SetDebugName(buffer.Get(), key);
            auto [_, status] = m_cache.emplace(key.data(), buffer);

            if (!status)
            {
                pBuffer = buffer.Detach(); //Give back to owner
                return LS::System::CreateFailCode(std::format("Could not add key: {}", key));
            }
            return LS::System::CreateSuccessCode();
        }

        [[nodiscard]]
        auto Get(std::string_view key) noexcept -> Nullable<Buffer>
        {
            if (!m_cache.contains(key.data()))
                return std::nullopt;

            return m_cache.at(key.data());
        }

        [[nodiscard]]
        auto Remove(std::string_view key) noexcept -> bool
        {
            if (!m_cache.contains(key.data()))
                return false;

            m_cache.erase(key.data());
            return true;
        }

        void SetCacheSize(CacheSize size) noexcept
        {
            m_limit = size;
        }

        [[nodiscard]]
        auto GetCacheSize() noexcept -> CacheSize
        {
            return m_limit;
        }

    private:
        CacheSize m_limit = 1024u * 1024u * 1024u * 3;
        Cache m_cache;
    };
}