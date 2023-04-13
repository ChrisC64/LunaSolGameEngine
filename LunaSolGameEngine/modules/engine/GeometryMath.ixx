export module MathLib:GeometryMath;

import Data.LSMath.Types;

export namespace LS::Math
{
    constexpr float LS_PI = 3.141592654f;
    constexpr float LS_PI2 = 6.283185307f;
    constexpr float LS_1DIVPI = 0.318309886f;
    constexpr float LS_1DIV2PI = 0.159154943f;
    constexpr float LS_PIDIV2 = 1.570796327f;
    constexpr float LS_PIDIV4 = 0.785398163f;

    consteval float ConvertToDegrees(float radians)
    {
        return radians * (180.0f / LS_PI);
    }
    
    consteval float ConvertToRadians(float degrees)
    {
        return degrees * (LS_PI / 180.0f);
    }
    
    consteval double ConvertToDegreesD(double radians)
    {
        return radians * (180.0 / LS_PI);
    }
    
    consteval double ConvertToRadiansD(double degrees)
    {
        return degrees * (LS_PI / 180.0);
    }
}