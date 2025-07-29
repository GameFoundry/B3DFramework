//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Debug/BsDebug.h"
#include "ECS/BsEntity.h"
#include "ECS/BsUtility.h"
#include "ECS/BsSparseSet.h"

#include <iterator>

namespace b3d::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

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

		void Add(Entity entity)
		{
			Iterator iterator = Super::AddInternal(entity, false);
			OnWasAdded(*iterator);
		}

		template<typename It>
		void Add(It first, It last, const TagType&)
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

		IteratorRange Each() { return IteratorRange({ Begin() }, { End() }); }
		ConstIteratorRange Each() const { return ConstIteratorRange({ Cbegin() }, { Cend() }); }

		ReverseIteratorRange ReverseEach() { return ReverseIteratorRange({ Rbegin() }, { Rend() }); }
		ConstReverseIteratorRange ReverseEach() const { return ConstReverseIteratorRange({ Crbegin() }, { Crend() }); }
	};

	template<typename Type>
	struct StorageForType<Type, std::enable_if_t<std::is_empty_v<Type>>>
	{
		using StorageType = TInheritConstFrom<TTagSparseSet<std::remove_const_t<Type>>, Type>;
	};

	/** @} */
} // namespace b3d::ecs
