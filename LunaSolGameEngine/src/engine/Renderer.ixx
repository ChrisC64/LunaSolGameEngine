module;
export module Engine.Renderer;
import <concepts>;
//import LSDataLib;

export namespace LS
{
    //class ICamera
    //{

    //};

    //class IScene
    //{
    //    virtual void Draw(const ICamera* camera) = 0;
    //    virtual void Update(float dt)
    //};

    //class IRenderer
    //{
    //    virtual void Clear(float r, float g, float b) noexcept = 0;
    //    virtual void DrawScene(const IScene* scene, const ICamera* camera) noexcept = 0;
    //    virtual void Present() noexcept = 0;
    //};

    template<class T>
    concept Camera = requires(T t)
    {
        /*{ t.Projection } -> std::same_as<LS::Mat4F>;
        { t.View } -> std::same_as<LS::Mat4F>;
        { t.Position } -> std::same_as<LS::Vec4F>;
        { t.Target } -> std::same_as<LS::Vec4F>;
        { t.Up } -> std::same_as<LS::Vec4F>;*/
        { t.NearZ } -> std::convertible_to<float>;
        { t.FarZ } -> std::convertible_to<float>;
        { t.Width } -> std::convertible_to<float>;
        { t.Height } -> std::convertible_to<float>;
    };

    template<class T, class C>
    concept Drawable = requires(T t, const C& c, float dt)
    {
        Camera<C>;
        { t.Draw(c) } noexcept;
        { t.Update(dt) } noexcept;
    };

    template<class T>
    concept ClearFrameBuffer = requires(T t, float r, float g, float b)
    {
        { t.Clear(r, g, b) };
    };

    //template<class T, class S, class C>
    //concept SceneDraw = requires(T t, const S& s, const C& c)
    //{
    //    Camera<C>;
    //    { t.DrawScene(s, c) };
    //};

    template<class T> 
    concept Present = requires(T t)
    {
        { t.Present() };
    };

    template<class T, class Camera>
    concept Renderer = requires
    {
        ClearFrameBuffer<T>;
        Present<T>;
    };

    //template<class U>
    //concept UpdateableFlt = requires(U u, float f)
    //{
    //    { u.Update(f) } noexcept;
    //};

    //template<class U>
    //concept UpdateableDbl = requires(U u, double d)
    //{
    //    { u.Update(d) } noexcept;
    //};
}