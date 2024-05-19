module;
#include <wrl/client.h>
#include <d3d11_4.h>
#include <unordered_map>
#include <string>
#include <string_view>
#include <cstdint>
#include <format>
#include <optional>
export module DX11Systems:DX11BufferCache;
import Engine.EngineCodes;
import Engine.Defines;
import Win32.ComUtils;

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
    using Keys = std::string;
    using Values = BufferContents;
    using Cache = std::unordered_map<Keys, Values>;

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
        auto Insert(std::string_view key, ComPtr<ID3D11Buffer> buffer) noexcept -> LS::System::ErrorCode
        {
            Utils::SetDebugName(buffer.Get(), key);
            BufferContents bc{ .Buffer = buffer };
            auto [_, status] = m_cache.emplace(key.data(), bc);

            if (!status)
                return LS::System::CreateFailCode(std::format("Could not add key: {}", key));
            return LS::System::CreateSuccessCode();
        }

        [[nodiscard]]
        auto Get(std::string_view key) noexcept -> Nullable<ComPtr<ID3D11Buffer>>
        {
            if (!m_cache.contains(key.data()) || m_cache.at(key.data()).State == LockState::LOCKED)
                return std::nullopt;

            return m_cache.at(key.data()).Buffer;
        }

        [[nodiscard]]
        auto Remove(std::string_view key) noexcept -> LS::System::ErrorCode
        {
            if (!m_cache.contains(key.data()))
            {
                return LS::System::CreateFailCode(std::format("Cannot remove buffer: {} It does not exist.", key));
            }

            if (m_cache.at(key.data()).State == LockState::LOCKED)
            {
                return LS::System::CreateFailCode(std::format("Buffer {} is currently locked.", key));
            }
            m_cache.erase(key.data());
            return LS::System::CreateSuccessCode();
        }

        [[nodiscard]]
        auto Lock(std::string_view key) noexcept -> Nullable<ComPtr<ID3D11Buffer>>
        {
            if (!m_cache.contains(key.data()) || m_cache.at(key.data()).State == LockState::LOCKED)
                return std::nullopt;

            return m_cache.at(key.data()).Buffer;
        }

        void Unlock(std::string_view key) noexcept
        {
            if (m_cache.contains(key.data()) && m_cache.at(key.data()).State == LockState::LOCKED)
            {
                m_cache.at(key.data()).State = LockState::UNLOCKED;
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

    private:
        std::uint64_t m_limit = 1024u * 1024u * 1024u * 3;
        Cache m_cache;
    };
}