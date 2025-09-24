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

	/**
	 * Returns a unique type identifier for the provided type. Note the identifiers may not remain constant across application runs.
	 *
	 * @note
	 * For consistent behaviour across plugins, make sure your type either inherits from IReflectable or provides a RTTIPlainType specialization, with a unique ID.
	 * Otherwise a runtime ID will be generated, which may be different for the same class across different dynamic libraries. This can also be worked around
	 * by explicitly specializing this method and correctly exporting it for the use by the dynamic libraries.
	 */
	template <typename T>
	TypeId B3DGetRuntimeTypeId()
	{
		if constexpr(std::is_base_of_v<IReflectable, std::remove_reference_t<std::remove_cv_t<T>>>)
			return (TypeId)T::GetRttiStatic()->GetRttiId();
		else if constexpr(B3DHasRTTIPlainTypeSpecialization<std::remove_reference_t<std::remove_cv_t<T>>>::value)
			return (TypeId)RTTIPlainType<T>::id;
		else
		{
			// Note: This will generate different IDs for the same type if called from dynamic libraries. In that case you must make
			// sure to export the specialization, so that each plugin doesn't try to create their own.
			static char const kTypeId = 0;
			return (u64)&kTypeId;
		}
	}

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

	/** @} */
} // namespace b3d
