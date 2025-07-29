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

	class EntitySparseSet : public TSparseSet<SparseSetDeletePolicy::SwapOnly>
	{
	public:
		using ElementType = Entity;
		using Super = TSparseSet<SparseSetDeletePolicy::SwapOnly>;
		using IteratorRange = TIteratorRange<Iterator>;
		using ConstIteratorRange = TIteratorRange<ConstIterator>;
		using ReverseIteratorRange = TIteratorRange<ReverseIterator>;
		using ConstReverseIteratorRange = TIteratorRange<ConstReverseIterator>;

		EntitySparseSet()
			:TSparseSet(B3DGetTypeHash<Entity>())
		{ }
		~EntitySparseSet() override = default;

		Entity Create()
		{
			Entity entity = Size() == GetFirstFreeElementPackedIndex() ? CreateEntity() : mPackedEntities[GetFirstFreeElementPackedIndex()];
			Iterator iterator = Super::AddInternal(entity, false);

			OnWasAdded(*iterator);
			return *iterator;
		}

		Entity Create(Entity hint)
		{
			if(hint != kInvalidEntity && hint != kNullEntity)
			{
				Entity entity(hint.GetIdentifier(), GetVersion(hint));
				if(entity == kInvalidEntity || GetPackedIndex(entity) >= GetFirstFreeElementPackedIndex())
				{
					Iterator iterator = Super::AddInternal(entity, false);
					OnWasAdded(*iterator);

					return *iterator;
				}
			}

			return Create();
		}

		void Clear() override
		{
			Iterator end = Super::Begin() + GetFirstFreeElementPackedIndex();
			for(auto it = Super::Begin(); it != end; ++it)
				OnWillRemove(*it);

			Super::ClearInternal();
			mNextEntityId = 0u;
		}

		IteratorRange Each() { return IteratorRange({ Begin() }, { Begin() + GetFirstFreeElementPackedIndex() }); }
		ConstIteratorRange Each() const { return ConstIteratorRange({ Cbegin() }, { Cbegin() + GetFirstFreeElementPackedIndex() }); }

		ReverseIteratorRange ReverseEach() { return ReverseIteratorRange({ Rbegin() }, { Rbegin() + (ReverseIterator::difference_type)GetFirstFreeElementPackedIndex() }); }
		ConstReverseIteratorRange ReverseEach() const { return ConstReverseIteratorRange({ Crbegin() }, { Crbegin() + (ReverseIterator::difference_type)GetFirstFreeElementPackedIndex() }); }

	private:
		using UnderlyingIterator = typename Super::Iterator;

		UnderlyingIterator AddInternal(Entity hint, bool forceAddAtEnd) override
		{
			return Super::Find(Create(hint));
		}

		Entity CreateEntity()
		{
			auto fnGetEntityChecked = [this]()
			{
				Entity entity(mNextEntityId, 0u);
				if(B3D_ENSURE(entity != kNullEntity))
					mNextEntityId++;

				return entity;
			};

			Entity entity = fnGetEntityChecked();
			while(Super::GetVersion(entity) != ((Entity)kInvalidEntity).GetVersion() && entity != kNullEntity)
				entity = fnGetEntityChecked();

			return entity;
		}

		Entity::IdentifierType mNextEntityId = 0u;
	};

	template<>
	struct StorageForType<Entity>
	{
		using StorageType = EntitySparseSet;
	};

	/** @} */
} // namespace b3d::ecs
