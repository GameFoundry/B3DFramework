//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "String/B3DHashedString.h"
#include "Reflection/B3DRTTIType.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	using TypeId = u64;
	using TypeHash = u64;

	/** Returns the type name for the class @p Type, using the 'pretty function' macro provided by many compilers. */
	template<typename Type>
	constexpr std::string_view B3DGetTypeNameFromPrettyFunction()
	{
		const std::string_view prettyFunctionString(B3D_PRETTY_FUNCTION);

#if B3D_COMPILER_MSVC
		// MSVC __FUNCSIG__ includes other template signatures before our template argument, so parse by function token.
		constexpr std::string_view kTypeNameStartToken = "B3DGetTypeNameFromPrettyFunction<";
		constexpr std::string_view kTypeNameEndToken = ">(void)";
		const auto typeNameStart = prettyFunctionString.find(kTypeNameStartToken) + kTypeNameStartToken.length();
		const auto typeNameEnd = prettyFunctionString.find(kTypeNameEndToken, typeNameStart);
		return prettyFunctionString.substr(typeNameStart, typeNameEnd - typeNameStart);
#else
		const auto typeNameStart = prettyFunctionString.find_first_not_of(' ', prettyFunctionString.find_first_of(B3D_PRETTY_FUNCTION_PREFIX) + 1);
		return prettyFunctionString.substr(typeNameStart, prettyFunctionString.find_last_of(B3D_PRETTY_FUNCTION_SUFFIX) - typeNameStart);
#endif
	}

	/** Computes a hash value for the current class name. Uses B3DGetTypeNameFromPrettyFunction() to compute the hash from. */
	template<typename Type, typename HashType = TypeHash>
	constexpr HashType B3DGetTypeHash()
	{
		constexpr std::string_view typeName = B3DGetTypeNameFromPrettyFunction<Type>();
		return THashedString<char, HashType>::CalculateHash(typeName);
	}

	/** Returns a unique type identifier for the provided type. */
	template <typename T>
	constexpr TypeId B3DGetRuntimeTypeId()
	{
		return B3DGetTypeHash<T>();
	}

	/** @} */
} // namespace b3d
