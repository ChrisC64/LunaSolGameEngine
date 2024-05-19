module;
#include <concepts>
#include <type_traits>

export module LSEDataLib:Resource;

import Engine.App;
import Engine.Defines;
import Engine.EngineCodes;
//TODO: Not sure if going to keep, my goal was to have something that keeps storage of all resource
// objects, but I'll probably work to keep separate resources like Buffers, Textures, Sound, etc. 
// instead of generalizing them. 
export namespace LS
{
    template <class Type>
        requires std::is_destructible<Type>
    struct LSResource
    {
        GuidUL Guid;
        Type Obj;
    };

    template <class T>
    concept LSLoader = requires(T t, GuidUL g)
    {
        { t.Load(t) } -> std::convertible_to<GuidUL>;
        t.Unload(g);
    };

    template <class T>
    class LSResourceManager
    {
    public:
        LSResourceManager() = default;
        ~LSResourceManager() = default;

        LSResourceManager(const LSResourceManager&) = delete;
        LSResourceManager& operator=(const LSResourceManager&) = delete;

        LSResourceManager(LSResourceManager&&) = default;
        LSResourceManager& operator=(LSResourceManager&&) = default;

        [[nodiscard]] auto GetSharedRef(GuidUL guid) const noexcept -> Nullable<SharedRef<T>>;
        [[nodiscard]] auto GetWeakRef(GuidUL guid) const noexcept -> Nullable<WeakRef<T>>;
        [[nodiscard]] auto Insert(GuidUL guid, T&& resource) noexcept -> LS::System::ErrorCode;
        [[nodiscard]] auto Insert(GuidUL guid, const T& resource) noexcept -> LS::System::ErrorCode;

        [[nodiscard]] auto Lock(GuidUL guid) noexcept -> WeakRef<T>;
        [[nodiscard]] auto TryLock(GuidUL guid) noexcept -> Nullable<WeakRef<T>>;
        [[nodiscard]] auto Unlock(GuidUL guid) noexcept -> LS::System::ErrorCode;
    };
}