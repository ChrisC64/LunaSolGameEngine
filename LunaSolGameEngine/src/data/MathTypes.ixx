module;
#include <exception>
#include <format>
#include <cmath>
#include <array>
#include <compare>

export module LSDataLib:MathTypes;
import Engine.Defines;

export namespace LS
{
    // Define Constants Here //
    constexpr float EPSILON_F = 0.0000001f;

    template<class T>
        requires BasicMathOperators<T> && IsNumerical<T>
    struct Vec2
    {
        T x{};
        T y{};

        [[nodiscard]] constexpr Vec2<T>& operator+=(const Vec2<T>& rhs) noexcept
        {
            x = x + rhs.x;
            y = y + rhs.y;
            return *this;
        }

        [[nodiscard]] constexpr Vec2<T>& operator-=(const Vec2<T>& rhs) noexcept
        {
            x = x - rhs.x;
            y = y - rhs.y;
            return *this;
        }

        [[nodiscard]] constexpr Vec2<T>& operator/=(T scalar) noexcept(scalar == 0)
        {
            if constexpr (scalar == 0)
            {
                throw std::invalid_argument("Cannot divide by 0!\n");
            }
            x = x / scalar;
            y = y / scalar;
            return *this;
        }

        [[nodiscard]] constexpr Vec2<T>& operator*=(T scalar) noexcept
        {
            x = x * scalar;
            y = y * scalar;
            return *this;
        }

        [[nodiscard]] constexpr bool operator<=(const Vec2<T>& rhs) noexcept
        {
            return x <= rhs.x && y <= rhs.y;
        }

        [[nodiscard]] constexpr bool operator>=(const Vec2<T>& rhs) noexcept
        {
            return x >= rhs.x && y >= rhs.y;
        }

        template<class T>
            requires std::is_floating_point<T>
        [[nodiscard]] constexpr auto operator<=>(const Vec2<T>& rhs) const noexcept
        {
            if (IsGreater(x, rhs.x) && IsGreater(y, rhs.y) )
            {
                return std::partial_ordering::greater;
            }
            else if (IsLess(x, rhs.x) && IsLess(y, rhs.y) )
            {
                return std::partial_ordering::less;
            }
            else if (IsEqual(x, rhs.x, EPSILON_F) && IsEqual(y, rhs.y, EPSILON_F))
            {
                std::partial_ordering::equivalent;
            }
            return std::partial_ordering::unordered;
        }

        [[nodiscard]] constexpr auto operator<=>(const Vec2<T>&) const noexcept = default;
    };

    template<class T>
        requires BasicMathOperators<T>&& IsNumerical<T>
    struct Vec3
    {
        T x{};
        T y{};
        T z{};

        [[nodiscard]] constexpr Vec3<T>& operator+=(const Vec3<T>& rhs) noexcept
        {
            x = x + rhs.x;
            y = y + rhs.y;
            z = z + rhs.z;
            return *this;
        }

        [[nodiscard]] constexpr Vec3<T>& operator-=(const Vec3<T>& rhs) noexcept
        {
            x = x - rhs.x;
            y = y - rhs.y;
            z = z - rhs.z;
            return *this;
        }

        [[nodiscard]] constexpr Vec3<T>& operator/=(T scalar) noexcept(scalar == 0)
        {
            if constexpr (scalar == 0)
            {
                throw std::invalid_argument("Cannot divide by 0!\n");
            }
            x = x / scalar;
            y = y / scalar;
            z = z / scalar;
            return *this;
        }

        [[nodiscard]] constexpr Vec3<T>& operator*=(T scalar) noexcept
        {
            x = x * scalar;
            y = y * scalar;
            z = z * scalar;
            return *this;
        }

        [[nodiscard]] constexpr bool operator<=(const Vec3<T>& rhs) noexcept
        {
            return x <= rhs.x && y <= rhs.y && z <= rhs.z;
        }

        [[nodiscard]] constexpr bool operator>=(const Vec3<T>& rhs) noexcept
        {
            return x >= rhs.x && y >= rhs.y && z >= rhs.z;
        }

        template<class T>
            requires std::is_floating_point<T>
        [[nodiscard]] constexpr auto operator<=>(const Vec3<T>& rhs) const noexcept
        {
            if (IsGreater(x, rhs.x) && IsGreater(y, rhs.y) && IsGreater(z, rhs.z))
            {
                return std::partial_ordering::greater;
            }
            else if (IsLess(x, rhs.x) && IsLess(y, rhs.y) && IsLess(z, rhs.z))
            {
                return std::partial_ordering::less;
            }
            else if (IsEqual(x, rhs.x, EPSILON_F) && IsEqual(y, rhs.y, EPSILON_F)
                && IsEqual(z, rhs.z, EPSILON_F) )
            {
                std::partial_ordering::equivalent;
            }
            return std::partial_ordering::unordered;
        }

        [[nodiscard]] constexpr auto operator<=>(const Vec3<T>&) const noexcept = default;
    };

    template<class T>
        requires BasicMathOperators<T>&& IsNumerical<T>
    struct Vec4
    {
        T x{};
        T y{};
        T z{};
        T w{};

        [[nodiscard]] constexpr Vec4<T>& operator+=(const Vec4<T>& rhs) noexcept
        {
            x = x + rhs.x;
            y = y + rhs.y;
            z = z + rhs.z;
            w = w + rhs.w;
            return *this;
        }

        [[nodiscard]] constexpr Vec4<T>& operator-=(const Vec4<T>& rhs) noexcept
        {
            x = x - rhs.x;
            y = y - rhs.y;
            z = z - rhs.z;
            w = w - rhs.w;
            return *this;
        }

        [[nodiscard]] constexpr Vec4<T>& operator/=(T scalar) noexcept(scalar == 0)
        {
            if constexpr (scalar == 0)
            {
                throw std::invalid_argument("Cannot divide by 0!\n");
            }
            x = x / scalar;
            y = y / scalar;
            z = z / scalar;
            w = w / scalar;
            return *this;
        }

        [[nodiscard]] constexpr Vec4<T>& operator*=(T scalar) noexcept
        {
            x = x * scalar;
            y = y * scalar;
            z = z * scalar;
            w = w * scalar;
            return *this;
        }

        [[nodiscard]] constexpr bool operator<=(const Vec4<T>& rhs) noexcept
        {
            return x <= rhs.x && y <= rhs.y && z <= rhs.z && w <= rhs.w;
        }

        [[nodiscard]] constexpr bool operator>=(const Vec4<T>& rhs) noexcept
        {
            return x >= rhs.x && y >= rhs.y && z >= rhs.z && w >= rhs.w;
        }

        template<class T>
            requires std::is_floating_point<T>
        [[nodiscard]] constexpr auto operator<=>(const Vec4<T>& rhs) const noexcept 
        {
            if (IsGreater(x, rhs.x) && IsGreater(y, rhs.y) && IsGreater(z, rhs.z) && IsGreater(w, rhs.w))
            {
                return std::partial_ordering::greater;
            }
            else if (IsLess(x, rhs.x) && IsLess(y, rhs.y) && IsLess(z, rhs.z) && IsLess(w, rhs.w))
            {
                return std::partial_ordering::less;
            }
            else if (IsEqual(x, rhs.x, EPSILON_F) && IsEqual(y, rhs.y, EPSILON_F) 
                && IsEqual(z, rhs.z, EPSILON_F) && IsEqual(w, rhs.w, EPSILON_F))
            {
                std::partial_ordering::equivalent;
            }
            return std::partial_ordering::unordered;
        }
        
        [[nodiscard]] constexpr auto operator<=>(const Vec4<T>&) const noexcept = default;

    };

    // Typedefs //
    using Vec2F = Vec2<float>;
    using Vec2D = Vec2<double>;
    using Vec2I = Vec2<int32_t>;
    using Vec2U = Vec2<uint32_t>;
    
    using Vec3F = Vec3<float>;
    using Vec3D = Vec3<double>;
    using Vec3I = Vec3<int32_t>;
    using Vec3U = Vec3<uint32_t>;

    using Vec4F = Vec4<float>;
    using Vec4D = Vec4<double>;
    using Vec4I = Vec4<int32_t>;
    using Vec4U = Vec4<uint32_t>;

    // Some Useful Functions - Maybe should be in Math lib //
    constexpr float abs(float a)
    {
        return a - ((a > 0) - (a < 0));
    }

    constexpr bool IsEqual(float a, float b, float epsilon)
    {
        return abs(a - b) <= epsilon;
    }
    
    constexpr bool IsLess(float a, float b, float epsilon)
    {
        return abs(a - b) < epsilon;
    }
    
    constexpr bool IsGreater(float a, float b, float epsilon)
    {
        return abs(a - b) > epsilon;
    }
    
    template<class T>
    [[nodiscard]] constexpr T Clamp(T result, T min, T max)
    {
        if constexpr (result < min)
        {
            return min;
        }
        else if constexpr (result > max)
        {
            return max;
        }
        else
        {
            return result;
        }
    }

    template<class T>
    [[nodiscard]] constexpr T Min(T lhs, T rhs)
    {
        return lhs < rhs ? lhs : rhs;
    }

    template<class T>
    [[nodiscard]] constexpr T Max(T lhs, T rhs)
    {
        return lhs > rhs ? lhs : rhs;
    }

    // Vec2 Operators //
    template<class T>
    [[nodiscard]] constexpr Vec2<T> operator+(const Vec2<T>& lh, const Vec2<T>& rh)
    {
        return Vec2<T>{.x = lh.x + rh.x, .y = lh.y + rh.y };
    }

    template<class T>
    [[nodiscard]] constexpr Vec2<T> operator-(const Vec2<T>& lh, const Vec2<T>& rh)
    {
        return Vec2<T>{.x = lh.x - rh.x, .y = lh.y - rh.y };
    }

    template<class T>
    [[nodiscard]] constexpr Vec2<T> operator*(const Vec2<T>& lh, const Vec2<T>& rh)
    {
        return Vec2<T>{.x = lh.x * rh.x, .y = lh.y * rh.y };
    }

    template<class T>
    [[nodiscard]] constexpr Vec2<T> operator*(const Vec2<T>& lh, float scalar)
    {
        return Vec2<T>{.x = lh.x * scalar, .y = lh.y * scalar };
    }

    template<class T>
    [[nodiscard]] constexpr Vec2<T> operator/(const Vec2<T>& lh, float scalar)
    {
        return Vec2<T>{.x = lh.x / scalar, .y = lh.y / scalar };
    }

    template<class T>
    [[nodiscard]] constexpr auto operator==(const Vec2<T>& lh, const Vec2<T>& rh)
    {
        return lh.x == rh.x && lh.y == rh.y;
    }
    
    // Vec3 Operators //
    template<class T>
    [[nodiscard]] constexpr Vec3<T> operator+(const Vec3<T>& lh, const Vec3<T>& rh)
    {
        return Vec3<T>{.x = lh.x + rh.x, .y = lh.y + rh.y, .z = lh.z + rh.z };
    }

    template<class T>
    [[nodiscard]] constexpr Vec3<T> operator-(const Vec3<T>& lh, const Vec3<T>& rh)
    {
        return Vec3<T>{.x = lh.x - rh.x, .y = lh.y - rh.y, .z = lh.z - rh.z };
    }

    template<class T>
    [[nodiscard]] constexpr Vec3<T> operator*(const Vec3<T>& lh, const Vec3<T>& rh)
    {
        return Vec3<T>{.x = lh.x * rh.x, .y = lh.y * rh.y, .z = lh.z * rh.z};
    }

    template<class T>
    [[nodiscard]] constexpr Vec3<T> operator*(const Vec3<T>& lh, T scalar)
    {
        return Vec3<T>{.x = lh.x * scalar, .y = lh.y * scalar, .z = lh.z * scalar };
    }

    template<class T>
    [[nodiscard]] constexpr Vec3<T> operator/(const Vec3<T>& lh, T scalar)
    {
        return Vec3<T>{.x = lh.x / scalar, .y = lh.y / scalar, .z = lh.z / scalar };
    }

    template<class T>
    [[nodiscard]] constexpr auto operator==(const Vec3<T>& lh, const Vec3<T>& rh)
    {
        return lh.x == rh.x && lh.y == rh.y && lh.z == rh.z;
    }
    
    // Vec4
    template<class T>
    [[nodiscard]] constexpr Vec4<T> operator+(const Vec4<T>& lh, const Vec4<T>& rh)
    {
        return Vec4<T>{.x = lh.x + rh.x, .y = lh.y + rh.y, .z = lh.z + rh.z, .w = lh.w + rh.w };
    }

    template<class T>
    [[nodiscard]] constexpr Vec4<T> operator-(const Vec4<T>& lh, const Vec4<T>& rh)
    {
        return Vec4<T>{.x = lh.x - rh.x, .y = lh.y - rh.y, .z = lh.z - rh.z, .w = lh.w - rh.w };
    }

    template<class T>
    [[nodiscard]] constexpr Vec4<T> operator*(const Vec4<T>& lh, const Vec4<T>& rh)
    {
        return Vec4<T>{.x = lh.x * rh.x, .y = lh.y * rh.y, .z = lh.z * rh.z, .w = lh.w * rh.w };
    }

    template<class T>
    [[nodiscard]] constexpr Vec4<T> operator*(const Vec4<T>& lh, T scalar)
    {
        return Vec4<T>{.x = lh.x * scalar, .y = lh.y * scalar, .z = lh.z * scalar, .w = lh.w * scalar };
    }

    template<class T>
    [[nodiscard]] constexpr Vec4<T> operator/(const Vec4<T>& lh, T scalar)
    {
        return Vec4<T>{.x = lh.x / scalar, .y = lh.y / scalar, .z = lh.z / scalar, .w = lh.z / scalar };
    }

    template<class T>
    [[nodiscard]] constexpr auto operator==(const Vec4<T>& lh, const Vec4<T>& rh)
    {
        return lh.x == rh.x && lh.y == rh.y && lh.z == rh.z && lh.w == rh.w;
    }

    template<class T>
        requires IsNumerical<T>
    struct Mat4
    {
        std::array<T, 16> Mat;
        constexpr T& at(size_t x, size_t y) {
            if (x * 4 + y >= Mat.size())
            {
                std::string msg = std::format("Out of range access for Matrix: {},{}\n", x, y);
                throw std::out_of_range(msg);
            }
            return Mat[x * 4 + y];
        }

        static constexpr Mat4 Identity()
        {
            Mat4<T> matrix;
            auto& mat = matrix.Mat;
            mat[0]  = 1; mat[1]  = 0; mat[2]  = 0; mat[3]  = 0;
            mat[4]  = 0; mat[5]  = 1; mat[6]  = 0; mat[7]  = 0;
            mat[8]  = 0; mat[9]  = 0; mat[10] = 1; mat[11] = 0;
            mat[12] = 0; mat[13] = 0; mat[14] = 0; mat[15] = 1;
            return matrix;
        }

        constexpr T& operator[](size_t idx)
        {
            if (idx >= Mat.size())
            {
                std::string msg = std::format("Out of range access for Matrix: {}\n", idx);
                throw std::out_of_range(msg);
            }
            return Mat[idx];
        }

        constexpr Vec4<T> Row(size_t row)
        {
            if (row > 3)
            {
                std::string msg = std::format("Out of range access for Matrix Row: {}\n", row);
                throw std::out_of_range(msg);
            }
            Vec4<T> out;
            out.x = Mat[row * 4];
            out.y = Mat[row * 4 + 1];
            out.z = Mat[row * 4 + 2];
            out.w = Mat[row * 4 + 3];

            return out;
        }

        constexpr Vec4<T> Col(size_t col)
        {
            if (col > 3)
            {
                std::string msg = std::format("Out of range access for Matrix Column: {}\n", col);
                throw std::out_of_range(msg);
            }
            Vec4<T> out;
            out.x = Mat[0 * 4 + col];
            out.y = Mat[1 * 4 + col];
            out.z = Mat[2 * 4 + col];
            out.w = Mat[3 * 4 + col];

            return out;
        }
    };
    
    template<class T>
        requires IsNumerical<T>
    struct Mat3
    {
        std::array<T, 9> Mat;

        constexpr T& at(uint32_t x, uint32_t y) 
        {
            if (x * 3 + y >= Mat.size())
            {
                std::string msg = std::format("Out of range access for Matrix: {},{}\n", x, y);
                throw std::out_of_range(msg);
            }
            return Mat[x * y];
        }

        static constexpr Mat3 Identity()
        {
            Mat3<T> matrix;
            auto& mat = matrix.Mat;
            mat[0] = 1; mat[1] = 0; mat[2] = 0;
            mat[3] = 0; mat[4] = 1; mat[5] = 0;
            mat[6] = 0; mat[7] = 0; mat[8] = 1;
            return mat;
        }

        constexpr T& operator[](size_t idx)
        {
            if (idx >= Mat.size())
            {
                std::string msg = std::format("Out of range access for Matrix: {}\n", idx);
                throw std::out_of_range(msg);
            }
            return Mat[idx];
        }

        constexpr Vec3<T> Row(size_t row)
        {
            if (row > 2)
            {
                std::string msg = std::format("Out of range access for Matrix Row: {}\n", row);
                throw std::out_of_range(msg);
            }
            Vec3<T> out;
            out.x = Mat[row * 3];
            out.y = Mat[row * 3 + 1];
            out.z = Mat[row * 3 + 2];

            return out;
        }

        constexpr Vec4<T> Col(size_t col)
        {
            if (col > 2)
            {
                std::string msg = std::format("Out of range access for Matrix Column: {}\n", col);
                throw std::out_of_range(msg);
            }
            Vec3<T> out;
            out.x = Mat[0 * 3 + col];
            out.y = Mat[1 * 3 + col];
            out.z = Mat[2 * 3 + col];

            return out;
        }
    };
    
    template<class T>
        requires IsNumerical<T>
    struct Mat2
    {
        std::array<T, 4> Mat;

        constexpr T& at(uint32_t x, uint32_t y) noexcept(x * 2 + y < Mat.size()) {
            if constexpr (x * 2 + y > Mat.size())
            {
                std::string msg = std::format("Out of range access for Matrix: {},{}\n", x, y);
                throw std::out_of_range(msg);
            }
            return Mat[x * y];
        }
        
        static constexpr Mat2 Identity()
        {
            Mat2<T> matrix;
            auto& mat = matrix.Mat;
            mat[0] = 1; mat[1] = 0;
            mat[2] = 0; mat[3] = 1;
            return mat;
        }

        constexpr T& operator[](size_t idx)
        {
            if (idx >= Mat.size())
            {
                std::string msg = std::format("Out of range access for Matrix: {}\n", idx);
                throw std::out_of_range(msg);
            }
            return Mat[idx];
        }

        constexpr Vec2<T> Row(size_t row)
        {
            if (row > 1)
            {
                std::string msg = std::format("Out of range access for Matrix Row: {}\n", row);
                throw std::out_of_range(msg);
            }
            Vec2<T> out;
            out.x = Mat[row];
            out.y = Mat[row + 1];

            return out;
        }

        constexpr Vec4<T> Col(size_t col)
        {
            if (col > 1)
            {
                std::string msg = std::format("Out of range access for Matrix Column: {}\n", col);
                throw std::out_of_range(msg);
            }
            Vec2<T> out;
            out.x = Mat[0 + col];
            out.y = Mat[2 + col];

            return out;
        }
    };

    using Mat4F = Mat4<float>;
    using Mat4I = Mat4<int>;
    using Mat3F = Mat3<float>;
    using Mat3I = Mat3<int>;
    using Mat2F = Mat3<float>;
    using Mat2I = Mat3<int>;
}