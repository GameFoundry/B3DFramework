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
		auto typeNameStart = prettyFunctionString.find_first_not_of(' ', prettyFunctionString.find_first_of(B3D_PRETTY_FUNCTION_PREFIX) + 1);
		return prettyFunctionString.substr(typeNameStart, prettyFunctionString.find_last_of(B3D_PRETTY_FUNCTION_SUFFIX) - typeNameStart);
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
		if constexpr(std::is_base_of_v<IReflectable, std::remove_reference_t<std::remove_cv_t<T>>>)
			return (TypeId)T::GetRttiStatic()->GetRttiId();
		else if constexpr(B3DHasRTTIPlainTypeSpecialization<std::remove_reference_t<std::remove_cv_t<T>>>::value)
			return (TypeId)RTTIPlainType<T>::id;
		else
			return B3DGetTypeHash<T>();
	}

	/** @} */
} // namespace b3d
