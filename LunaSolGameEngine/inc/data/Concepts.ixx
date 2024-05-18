module;
#include <concepts>
#include <type_traits>

export module LSEDataLib:Concepts;

export namespace LS
{
    template <class T>
    concept IsEnum = std::is_enum_v<T>;

    template <class T>
    concept IsIntegral = std::is_integral_v<T>;
	template <class T>
	concept IsSigned = IsIntegral<T> && std::is_signed_v<T>;
	template <class T>
	concept IsUnsigned = IsIntegral<T> && std::is_unsigned_v<T>;
	template <class T>
	concept IsFloatingPoint = std::is_floating_point_v<T>;

	template <class T>
	concept IsNumerical = std::is_integral_v<T> || std::is_floating_point_v<T>;

	template <class T>
	concept Addable = requires (T x) { x + x; };
	template <class T>
	concept Subtractable = requires (T x) { x - x; };
	template <class T>
	concept Multiplicative = requires (T x) { x* x; };
	template <class T>
	concept Divisible = requires (T x) { x / x; };
	template <class T>
	concept BasicMathOperators = requires {
		{ Addable<T> };
		{ Subtractable<T> };
		{ Multiplicative<T> };
		{ Divisible<T> };
	};

	template <class T>
	concept DestructibleNoThrow = std::is_nothrow_destructible_v<T>;
}