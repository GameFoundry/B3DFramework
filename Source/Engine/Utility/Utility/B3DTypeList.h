//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Metaprogramming
	 *  @{
	 */

	/** Identifies a list of types as a unique type, primarily for meta-programming purposes. */
	template<typename... Types>
	struct TTypeList
	{
		using Type = TTypeList;
		static constexpr u32 Size = sizeof...(Types);
	};

	template<typename, typename>
	struct TTypeListIndexOfHelper;

	template<typename Type, typename First, typename... Other>
	struct TTypeListIndexOfHelper<Type, TTypeList<First, Other...>>
	{
		static constexpr u32 Value = 1u + TTypeListIndexOfHelper<Type, TTypeList<Other...>>::Value;
	};

	template<typename Type, typename... Other>
	struct TTypeListIndexOfHelper<Type, TTypeList<Type, Other...>>
	{
		static constexpr u32 Value = 0u;
	};

	template<typename Type>
	struct TTypeListIndexOfHelper<Type, TTypeList<>>
	{
		static constexpr u32 Value = 0u;
	};

	/** Returns an index of a type within a list of types. The list of types must not contain duplicate types. */
	template<typename Type, typename TypeList>
	constexpr u32 TTypeListIndexOf = TTypeListIndexOfHelper<Type, TypeList>::Value;

	template<u32, typename>
	struct TTypeListElementAtHelper;

	template<u32 Index, typename First, typename... Other>
	struct TTypeListElementAtHelper<Index, TTypeList<First, Other...>> : TTypeListElementAtHelper<Index - 1u, TTypeList<Other...>>
	{ };

	template<typename First, typename... Other>
	struct TTypeListElementAtHelper<0u, TTypeList<First, Other...>>
	{
		using Type = First;
	};

	/** Returns a type at the specified index within a list of types. */
	template<u32 Index, typename List>
	using TTypeListElementAt = typename TTypeListElementAtHelper<Index, List>::Type;

	/** @} */
}
