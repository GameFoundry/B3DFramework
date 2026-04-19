//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DUtility.h"
#include "ECS/B3DSparseSet.h"

#include <iterator>

namespace b3d::ecs
{
	/** @addtogroup ECS
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	/**
	 * Storage used for storing tags associated with entities. Tags are components that are empty (have no size), and therefore don't require
	 * a separate payload array. Rather just existance of an entity in this storage is enough to let us know that there's a tag associated
	 * with the entity.
	 */
	template<typename TagType>
	class TTagSparseSet : public TSparseSet<SparseSetDeletePolicy::SwapAndErase>
	{
	public:
		using ElementType = TagType;
		using Super = TSparseSet<SparseSetDeletePolicy::SwapAndErase>;
		using IteratorRange = TIteratorRange<TMultiIteratorAdapter<Iterator>>;
		using ConstIteratorRange = TIteratorRange<TMultiIteratorAdapter<ConstIterator>>;
		using ReverseIteratorRange = TIteratorRange<TMultiIteratorAdapter<ReverseIterator>>;
		using ConstReverseIteratorRange = TIteratorRange<TMultiIteratorAdapter<ConstReverseIterator>>;

		TTagSparseSet()
			:TSparseSet(B3DGetTypeHash<TagType>())
		{ }

		~TTagSparseSet() override = default;

		/** Registers an entity with a tag. */
		void Add(Entity entity)
		{
			Iterator iterator = Super::AddInternal(entity, false);
			OnWasAdded(*iterator);
		}

		/** Registers a range of entities with a tag. */
		template<typename It>
		void Add(It first, It last, const TagType&) // Last parameter is ignored, but we're keeping for the sake of having a common interface across all storages
		{
			for(It it = first; it != last; ++it)
			{
				Iterator newEntityIterator = Super::AddInternal(*it, true);
				OnWasAdded(*newEntityIterator);
			}
		}

		void Get(Entity entity) const
		{
			B3D_ASSERT(false && "This method is only available for type deduction purposes and should not be called.");
		}

		/** Allows easy iteration over all tags using a range for loop. */
		IteratorRange Each() { return IteratorRange({ Begin() }, { End() }); }
		ConstIteratorRange Each() const { return ConstIteratorRange({ Cbegin() }, { Cend() }); }

		/** Allows easy iteration over all tags using a range for loop, in reverse order. */
		ReverseIteratorRange ReverseEach() { return ReverseIteratorRange({ Rbegin() }, { Rend() }); }
		ConstReverseIteratorRange ReverseEach() const { return ConstReverseIteratorRange({ Crbegin() }, { Crend() }); }
	};

	/** @} */

	/** @addtogroup ECS-Internal
	 *  @{
	 */

	template<typename Type>
	struct StorageForType<Type, std::enable_if_t<std::is_empty_v<Type>>>
	{
		using StorageType = TInheritConstFrom<TTagSparseSet<std::remove_const_t<Type>>, Type>;
	};

	/** @} */
} // namespace b3d::ecs
