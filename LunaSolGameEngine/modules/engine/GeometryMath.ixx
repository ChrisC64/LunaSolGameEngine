export module MathLib:GeometryMath;

import Data.LSMathTypes;

export namespace LS::Math
{
    constexpr float LS_PI = 3.141592654f;
    constexpr float LS_PI2 = 6.283185307f;
    constexpr float LS_1DIVPI = 0.318309886f;
    constexpr float LS_1DIV2PI = 0.159154943f;
    constexpr float LS_PIDIV2 = 1.570796327f;
    constexpr float LS_PIDIV4 = 0.785398163f;
    constexpr float LS_RADIANS = LS_PI / 180.0f;
    constexpr float LS_DEGREES = 180 / LS_PI;

    consteval float ConstEvalToDegrees(float radians)
    {
        return radians * LS_DEGREES;
    }
    
    consteval float ConstEvalToRadians(float degrees)
    {
        return degrees * LS_RADIANS;
    }
    
    consteval double ConstEvalToDegreesDouble(double radians)
    {
        return radians * LS_DEGREES;
    }
    
    consteval double ConstEvalToRadiansDouble(double degrees)
    {
        return degrees * LS_RADIANS;
    }
    
    constexpr float ToDegrees(float radians)
    {
        return radians * LS_DEGREES;
    }
    
    constexpr float ToRadians(float degrees)
    {
        return degrees * LS_RADIANS;
    }
    
    constexpr double ToDegreesDouble(double radians)
    {
        return radians * LS_DEGREES;
    }
    
    constexpr double ToRadiansDouble(double degrees)
    {
        return degrees * LS_RADIANS;
    }
}