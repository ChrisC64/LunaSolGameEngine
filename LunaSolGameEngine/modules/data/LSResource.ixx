module;

export module Data.LSResource;

import Data.LSDataTypes;
import Engine.App;

export namespace LS::Resource
{
    template <class Type>
    struct Resource
    {
        GuidUL Guid;
        Type Obj;
    };

    template <class T, class Res = Resource<T>>
    class ResourceManager
    {
    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        ResourceManager(const&) = delete;
        ResourceManager& operator=(const&) = delete;

        ResourceManager(&&) = default;
        ResourceManager& operator=(&&) = default;

        [[nodiscard]]
        auto Get(GuidUL guid) const noexcept -> Nullable<Ref<T>>;

        [[nodiscard]]
        auto Set(GuidUL guid, Ref<T> resource) noexcept -> LS::System::ErrorCode;
        [[nodiscard]]
        auto Set(GuidUL guid, T&& resource) noexcept -> LS::System::ErrorCode;
        [[nodiscard]]
        auto Set(GuidUL guid, const T& resource) noexcept -> LS::System::ErrorCode;

        [[nodiscard]]
        auto Lock(GuidUL guid) noexcept -> Ref<T>;
        [[nodiscard]]
        auto TryLock(GuidUL guid) noexcept -> Nullable<Ref<T>>;
        void Unlock(GuidUL guid) noexcept;
    };
}