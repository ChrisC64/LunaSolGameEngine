module;
#include <array>
#include <string>
export module GeometryGenerator;
import LSEDataLib;

export namespace LS::Geo::Generator
{
    template<class T>
        requires LS::IsNumerical<T>
    [[nodiscard]] constexpr auto CreateQuadVertices(T size, bool isFloor) -> std::array<Vec3<T>, 4>
    {
        std::array<Vec3<T>, 4> out;
        // Floor / Z Plane
        if (isFloor)
        {
            out[0] = { .x = -size, .y = -size, .z = 0 };
            out[1] = { .x = -size, .y = +size, .z = 0 };
            out[2] = { .x = +size, .y = +size, .z = 0 };
            out[3] = { .x = +size, .y = -size, .z = 0 };
        }
        else
        {
            out[0] = { .x = -size, .y = 0, .z = -size };
            out[1] = { .x = -size, .y = 0, .z = +size };
            out[2] = { .x = +size, .y = 0, .z = +size };
            out[3] = { .x = +size, .y = 0, .z = -size };
        }
        return out;
    }

    /**
     * @brief Constructs a vertex array of positions for a cube. The cube generated has 8 vertices
     * @param size size of cube to make, generally want to scale with a vector and leave with default
     * @return Cube with 8 vertices
    */
    template <class T>
        requires LS::IsNumerical<T>
    [[nodiscard]] constexpr auto CreateCubeVertices(T size) -> std::array<Vec3<float>, 8>
    {
        std::array<Vec3<T>, 8> out;
        // Assuming +Z is away from screen and -Z is towward (LH)
        // -Z axis means this will be the plane behind (back face)
        out[0] = { .x = -size, .y = -size, .z = -size };// BBL
        out[1] = { .x = -size, .y = +size, .z = -size };// BTL
        out[2] = { .x = +size, .y = +size, .z = -size };// BTR
        out[3] = { .x = +size, .y = -size, .z = -size };// BBR
        // +Z axis means this will be the plane in front (front face)
        out[4] = { .x = -size, .y = -size, .z = +size };// FBL
        out[5] = { .x = -size, .y = +size, .z = +size };// FTL
        out[6] = { .x = +size, .y = +size, .z = +size };// FTR
        out[7] = { .x = +size, .y = -size, .z = +size };// FBR

        return out;
    }

    /**
     * @brief Returns the Index array to the cube generated by @link LS::Geo::Generator::CreateCubeVertices
     * @return An array of the 36 integers needed to construct the connection with the cube object's vertices
    */
    [[nodiscard]] constexpr auto CreateCubeIndexArray() -> std::array<uint32_t, 36>
    {
        std::array<uint32_t, 36> indexList;

        // Back face //
        indexList[0] = 0;  indexList[1] = 1;   indexList[2] = 2;
        indexList[3] = 0;  indexList[4] = 2;   indexList[5] = 3;
        // Front face //
        indexList[6] = 5;  indexList[7] = 4;   indexList[8] = 7;
        indexList[9] = 5;  indexList[10] = 7;  indexList[11] = 6;
        // Top face //
        indexList[12] = 5; indexList[13] = 2; indexList[14] = 1;
        indexList[15] = 6; indexList[16] = 2; indexList[17] = 5;
        // Bottom face //
        indexList[18] = 4; indexList[19] = 0; indexList[20] = 3;
        indexList[21] = 4; indexList[22] = 3; indexList[23] = 7;
        // Left side face//
        indexList[24] = 4; indexList[25] = 5; indexList[26] = 1;
        indexList[27] = 4; indexList[28] = 1; indexList[29] = 0;
        // Right side face //
        indexList[30] = 3; indexList[31] = 2; indexList[32] = 6;
        indexList[33] = 3; indexList[34] = 6; indexList[35] = 7;

        return indexList;
    }

    [[nodiscard]] constexpr auto CreateQuadIndexArray() -> std::array<uint32_t, 6>
    {
        std::array<uint32_t, 6> out;
        out[0] = 0;  out[1] = 1;   out[2] = 2;
        out[3] = 0;  out[4] = 2;   out[5] = 3;
        return out;
    }

    template <class T>
    [[nodiscard]] constexpr auto CreateCubeVertsAndIndices(T size) -> std::pair<std::array<Vec3<T>, 8>, std::array<uint32_t, 36>>
    {
        auto verts = CreateCubeVertices(size);
        auto indices = CreateCubeIndexArray();

        return { verts, indices };
    }
    
    template <class T>
    [[nodiscard]] constexpr auto CreateQuadVertsAndIndices(T size, bool isFloor) -> std::pair<std::array<Vec3<T>, 4>, std::array<uint32_t, 6>>
    {
        auto verts = CreateQuadVertices(size, isFloor);
        auto indices = CreateQuadIndexArray();

        return { verts, indices };
    }
}