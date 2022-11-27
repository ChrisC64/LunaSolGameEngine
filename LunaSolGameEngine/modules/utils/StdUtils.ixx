module;
#include "LSEFramework.h"

export module Util.StdUtils;
import Data.LSConcepts;

// Set of functions to help with std stuff
export namespace LS::Utils
{
	template <class Enum>
	constexpr size_t HashEnum(const Enum& e) noexcept requires IsEnum<Enum>
	{
		return static_cast<std::size_t>(e);
	}

	template<typename Enum>
	constexpr auto ToIntegral(Enum e) noexcept
	{
		return static_cast<typename std::underlying_type_t<Enum>>(e);
	}

	template<typename Enum, typename T>
	constexpr auto ToEnum(T t) noexcept requires IsEnum<Enum> && IsIntegral<T>
	{
		static_assert(std::is_enum_v<Enum> && std::is_integral_v<T>,
			"Can only use with Enums and integral types.");
		return static_cast<Enum>(t);
	}

}