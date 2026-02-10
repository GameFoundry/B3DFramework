//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include <type_traits>

namespace b3d::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Compile-time grouping of related ECS tag types for serialization.
	 *
	 * Each tag's position in the parameter list determines its bit index in the serialized
	 * integer representation. When serialized, a TagGroup produces a single integer where
	 * bit N is set if the Nth tag is present on the entity.
	 *
	 * @tparam Storage  Unsigned integer type for the serialized bitfield (u8, u16, or u32).
	 *					Must be large enough to hold one bit per tag.
	 * @tparam Tags		Empty struct types representing tags. All types must satisfy
	 *					std::is_empty_v.
	 */
	template<typename Storage, typename... Tags>
	struct TagGroup
	{
		using StorageType = Storage;
		static constexpr u32 kTagCount = sizeof...(Tags);

		static_assert(kTagCount > 0, "TagGroup must contain at least one tag");
		static_assert(kTagCount <= sizeof(StorageType) * 8, "Too many tags for the specified storage type. Use a larger type.");
		static_assert((std::is_empty_v<Tags> && ...), "All TagGroup types must be empty tag types");
		static_assert(std::is_unsigned_v<StorageType>, "StorageType must be an unsigned integer type");
	};

	/** Type trait to detect TagGroup specializations at compile time. */
	template<typename T>
	struct IsTagGroup : std::false_type {};

	template<typename Storage, typename... Tags>
	struct IsTagGroup<TagGroup<Storage, Tags...>> : std::true_type {};

	/** @} */
} // namespace b3d::ecs
