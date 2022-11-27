export module Engine.LSScene;

import Engine.LSCamera;
export namespace LS
{
    class Scene
    {
    public:
        Scene() = default;
        ~Scene() = default;

        void Update(double deltaTime);
        void Draw(const LSCamera& camera);
    };
}