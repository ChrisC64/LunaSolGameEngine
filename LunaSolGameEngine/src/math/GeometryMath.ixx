export module MathLib:GeometryMath;

import LSDataLib;

export namespace LS::Math
{
    constexpr double LS_PI = 3.141592654f;
    constexpr double LS_PI2 = 6.283185307f;
    constexpr double LS_1DIVPI = 0.318309886f;
    constexpr double LS_1DIV2PI = 0.159154943f;
    constexpr double LS_PIDIV2 = 1.570796327f;
    constexpr double LS_PIDIV4 = 0.785398163f;
    constexpr double LS_RADIANS = LS_PI / 180.0f;
    constexpr double LS_DEGREES = 180 / LS_PI;

    consteval double ConstEvalToDegrees(float radians)
    {
        return radians * LS_DEGREES;
    }

    consteval double ConstEvalToRadians(float degrees)
    {
        return degrees * LS_RADIANS;
    }

    constexpr double ToDegrees(double radians)
    {
        return radians * LS_DEGREES;
    }

    constexpr double ToRadians(double degrees)
    {
        return degrees * LS_RADIANS;
    }
}