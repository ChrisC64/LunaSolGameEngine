module;
#include "LSEFramework.h"
export module Data.LSMath.Types;
import Data.LSConcepts;

export namespace LS::Types
{
	template<class T>
		requires BasicMathOperators<T>
	struct Point2D
	{
		T x{};
		T y{};

		[[nodiscard]] constexpr Point2D<T>& operator+=(const Point2D<T>& rhs) noexcept
		{
			x = x + rhs.x;
			y = y + rhs.y;
			return *this;
		}

		[[nodiscard]] constexpr Point2D<T>& operator-=(const Point2D<T>& rhs) noexcept
		{
			x = x - rhs.x;
			y = y - rhs.y;
			return *this;
		}

		[[nodiscard]] constexpr Point2D<T>& operator/=(float scalar) noexcept
		{
			x = x / scalar;
			y = y / scalar;
			return *this;
		}

		[[nodiscard]] constexpr Point2D<T>& operator*=(float scalar) noexcept
		{
			x = x * scalar;
			y = y * scalar;
			return *this;
		}

		[[nodiscard]] constexpr bool operator<=(const Point2D<T>& rhs) noexcept
		{
			return x <= rhs.x && y <= rhs.y;
		}

		[[nodiscard]] constexpr bool operator>=(const Point2D<T>& rhs) noexcept
		{
			return x >= rhs.x && y >= rhs.y;
		}

		[[nodiscard]] constexpr auto operator<=>(const Point2D<T>&) const noexcept = default;
	};

	// Common Declarations //
	using PointF = Point2D<float>;
	using PointD = Point2D<double>;
	using PointI = Point2D<int32_t>;
	using pointU = Point2D<uint32_t>;

	template<class T>
	[[nodiscard]] constexpr T Clamp(T result, T min, T max)
	{
		return result < min ? min : result > max ? max : result;
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

	template<class T>
	[[nodiscard]] constexpr Point2D<T> operator+(const Point2D<T>& lh, const Point2D<T>& rh)
	{
		return Point2D<T>{.x = lh.x + rh.x, .y = lh.y + rh.y };
	}

	template<class T>
	[[nodiscard]] constexpr Point2D<T> operator-(const Point2D<T>& lh, const Point2D<T>& rh)
	{
		return Point2D<T>{.x = lh.x - rh.x, .y = lh.y - rh.y };
	}

	template<class T>
	[[nodiscard]] constexpr Point2D<T> operator*(const Point2D<T>& lh, const Point2D<T>& rh)
	{
		return Point2D<T>{.x = lh.x * rh.x, .y = lh.y * rh.y };
	}

	template<class T>
	[[nodiscard]] constexpr Point2D<T> operator*(const Point2D<T>& lh, float scalar)
	{
		return Point2D<T>{.x = lh.x * scalar, .y = lh.y * scalar };
	}

	template<class T>
	[[nodiscard]] constexpr Point2D<T> operator/(const Point2D<T>& lh, float scalar)
	{
		return Point2D<T>{.x = lh.x / scalar, .y = lh.y / scalar };
	}

	template<class T>
	[[nodiscard]] constexpr auto operator==(const Point2D<T>& lh, const Point2D<T>& rh)
	{
		return lh.x == rh.x && lh.y == rh.y;
	}
}