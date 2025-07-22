//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Debug/BsDebug.h"
#include "ECS/BsEntity.h"
#include "ECS/BsUtility.h"
#include "ECS/BsSparseSet.h"
#include "BsUtility.h"

#include <iterator>

#include "BsEntityStorage.h"

namespace b3d::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	template<typename IteratorType, typename OwnedStorageType, typename IncludedStorageType>
	struct TGroupIteratorAdapter {};

	template<typename IteratorType, typename... OwnedStorageTypes, typename... IncludedStorageTypes>
	struct TGroupIteratorAdapter<IteratorType, TOwnedTypes<OwnedStorageTypes...>, TIncludedTypes<IncludedStorageTypes...>>
	{
		using iterator_type = IteratorType;
		using value_type = decltype(std::tuple_cat(std::make_tuple(*std::declval<IteratorType>()), GetAsTuple<OwnedStorageTypes>(nullptr, {})..., GetAsTuple<IncludedStorageTypes>(nullptr, {})...));
		using pointer = TPointerToTemporary<value_type>;
		using reference = value_type;
		using difference_type = typename std::iterator_traits<IteratorType>::difference_type;
		using iterator_category = std::input_iterator_tag;

		constexpr TGroupIteratorAdapter() = default;
		constexpr TGroupIteratorAdapter(IteratorType iterator, std::tuple<OwnedStorageTypes*..., IncludedStorageTypes*> ownedAndIncludedTypeStorage)
			: mIterator(iterator), mOwnedAndIncludedTypeStorage(std::move(ownedAndIncludedTypeStorage))
		{ }

		constexpr TGroupIteratorAdapter& operator++()
		{
			++mIterator;
			return *this;
		}

		constexpr pointer operator->() const
		{
			return operator*();
		}

		constexpr reference operator*() const
		{
			return std::tuple_cat(
				std::make_tuple(*mIterator),
				GetOwnedStorageElementTuple<OwnedStorageTypes>(*std::get<OwnedStorageTypes*>(mOwnedAndIncludedTypeStorage))...,
				GetAsTuple(std::get<IncludedStorageTypes*>(mOwnedAndIncludedTypeStorage), *mIterator)...);
		}

		constexpr iterator_type GetUnderlyingIterator() const
		{
			return mIterator;
		}

		template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
		friend constexpr bool operator==(const TGroupIteratorAdapter<LeftIteratorTypes...>&, const TGroupIteratorAdapter<RightIteratorTypes...>&);

	private:
		template<typename OwnedStorageType>
		auto GetOwnedStorageElementTuple(OwnedStorageType& storage)
		{
			if constexpr(std::is_void_v<typename OwnedStorageType::ValueType>)
				return std::make_tuple();
			else
				return GetAsTuple(storage[mIterator.Index()]);
		}

		IteratorType mIterator;
		std::tuple<OwnedStorageTypes*..., IncludedStorageTypes*> mOwnedAndIncludedTypeStorage;
	};

	template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
	constexpr bool operator==(const TGroupIteratorAdapter<LeftIteratorTypes...>& lhs, const TGroupIteratorAdapter<RightIteratorTypes...>& rhs)
	{
		return lhs.GetUnderlyingIterator() == rhs.GetUnderlyingIterator();
	}

	template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
	constexpr bool operator!=(const TGroupIteratorAdapter<LeftIteratorTypes...>& lhs, const TGroupIteratorAdapter<RightIteratorTypes...>& rhs)
	{
		return !(lhs == rhs);
	}

	template<u32 OwnedTypeCount, u32 IncludedTypeCount, u32 ExcludedTypeCount>
	struct TGroupInternals
	{
		template<typename OwnedAndIncludedTypes, typename ExcludedTypes>
		TGroupInternals(const std::tuple<OwnedAndIncludedTypes...>& ownedAndIncludedTypes, const std::tuple<ExcludedTypes>& excludedTypes)
			: mIncludedTypeStorage(std::apply([](auto*... storage) { return std::array<SparseSet*, OwnedTypeCount + IncludedTypeCount>(storage...); }, ownedAndIncludedTypes))
			, mExcludedTypeStorage(std::apply([](auto*... storage) { return std::array<SparseSet*, ExcludedTypeCount>(storage...); }), excludedTypes)
		{
			u32 eventHandleIndex = 0;
			std::apply([this, &eventHandleIndex](auto*... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage->OnDidAdd.Connect(&TGroupInternals::TryAddEntryToGroupAfterAdd),
					mEventHandles[eventHandleIndex++] = storage->OnWillRemove.Connect(&TGroupInternals::TryRemoveFromGroup)), ...);
			}, ownedAndIncludedTypes);

			std::apply([this, &eventHandleIndex](auto*... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage->OnDidAdd.Connect(&TGroupInternals::TryRemoveFromGroup),
					mEventHandles[eventHandleIndex++] = storage->OnWillRemove.Connect(&TGroupInternals::TryAddEntryToGroupBeforeRemove)), ...);
			});
			
			for(auto entry : mIncludedTypeStorage[0])
				TryAddEntryToGroupAfterAdd(entry);
		}

		~TGroupInternals()
		{
			for(const auto& entry : mEventHandles)
				entry.Disconnect();
		}

		u64 Size() const { return mNextIndex; }

		template<u32 Index>
		SparseSet* GetStorage()
		{
			if constexpr(Index < (OwnedTypeCount + IncludedTypeCount))
				return mIncludedTypeStorage[Index];
			else
				return mExcludedTypeStorage[Index - (OwnedTypeCount + IncludedTypeCount)];
		}
		
	private:
		void SwapEntry(u64 packedIndex, Entity entity)
		{
			for(u32 storageIndex = 0; storageIndex < OwnedTypeCount; ++storageIndex)
				mIncludedTypeStorage[storageIndex]->Swap(mIncludedTypeStorage[packedIndex], entity);
		}

		void TryAddEntryToGroupAfterAdd(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity, index = mNextIndex](auto* firstStorage, auto*... otherStorage)
			{
				return firstStorage->Contains(entity) && firstStorage->GetPackedIndex(entity) >= index && (otherStorage->Contains(entity) && ...);
			});

			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage) { return (!storage->Contains(entity) && ...); });

			if(inclusiveFilterPassed && exclusiveFilterPassed)
				SwapEntry(mNextIndex++, entity);
		}

		void TryAddEntryToGroupBeforeRemove(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity, index = mNextIndex](auto* firstStorage, auto*... otherStorage)
			{
				return firstStorage->Contains(entity) && firstStorage->GetPackedIndex(entity) >= index && (otherStorage->Contains(entity) && ...);
			});

			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage)
			{
				return (0u + ... + storage->Contains(entity)) == 1u;
			});

			if(inclusiveFilterPassed && exclusiveFilterPassed)
				SwapEntry(mNextIndex++, entity);
		}

		void TryRemoveFromGroup(Entity entity)
		{
			if(mIncludedTypeStorage[0]->Contains(entity) && mIncludedTypeStorage->GetPackedIndex(entity) < mNextIndex)
				SwapEntry(mNextIndex--, entity);
		}

		std::array<SparseSet*, OwnedTypeCount + IncludedTypeCount> mIncludedTypeStorage { };
		std::array<SparseSet*, ExcludedTypeCount> mExcludedTypeStorage { };
		std::array<HEvent, (OwnedTypeCount + IncludedTypeCount + ExcludedTypeCount) * 2> mEventHandles;
		u64 mNextIndex = 0;
	};

	template<u32 IncludedTypeCount, u32 ExcludedTypeCount>
	struct TGroupInternals<0, IncludedTypeCount, ExcludedTypeCount>
	{
		template<typename IncludedTypes, typename ExcludedTypes>
		TGroupInternals(const std::tuple<IncludedTypes...>& includedTypes, const std::tuple<ExcludedTypes>& excludedTypes)
			: mIncludedTypeStorage(std::apply([](auto*... storage) { return std::array<SparseSet*, IncludedTypeCount>(storage...); }, includedTypes))
			, mExcludedTypeStorage(std::apply([](auto*... storage) { return std::array<SparseSet*, ExcludedTypeCount>(storage...); }), excludedTypes)
		{
			u32 eventHandleIndex = 0;
			std::apply([this, &eventHandleIndex](auto*... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage->OnDidAdd.Connect(&TGroupInternals::TryAddEntryToGroupAfterAdd),
					mEventHandles[eventHandleIndex++] = storage->OnWillRemove.Connect(&TGroupInternals::TryRemoveFromGroup)), ...);
			}, includedTypes);

			std::apply([this, &eventHandleIndex](auto*... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage->OnDidAdd.Connect(&TGroupInternals::TryRemoveFromGroup),
					mEventHandles[eventHandleIndex++] = storage->OnWillRemove.Connect(&TGroupInternals::TryAddEntryToGroupBeforeRemove)), ...);
			});
			
			for(auto entry : mIncludedTypeStorage[0])
				TryAddEntryToGroupAfterAdd(entry);
		}

		~TGroupInternals()
		{
			for(const auto& entry : mEventHandles)
				entry.Disconnect();
		}

		EntitySparseSet& GetGroupStorage() { return mGroupEntities; }
		const EntitySparseSet& GetGroupStorage() const { return mGroupEntities; }

		template<u32 Index>
		SparseSet* GetStorage()
		{
			if constexpr(Index < IncludedTypeCount)
				return mIncludedTypeStorage[Index];
			else
				return mExcludedTypeStorage[Index - IncludedTypeCount];
		}
		
	private:
		void TryAddEntryToGroupAfterAdd(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity](auto*... storage) { return (storage->Contains(entity) && ...); });
			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage) { return (!storage->Contains(entity) && ...); });

			if(!mGroupEntities.Contains(entity) && inclusiveFilterPassed && exclusiveFilterPassed)
				mGroupEntities.Add(entity);
		}

		void TryAddEntryToGroupBeforeRemove(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity](auto*... storage) { return (storage->Contains(entity) && ...); });
			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage) { return (0u + ... + storage->Contains(entity)) == 1u; });

			if(!mGroupEntities.Contains(entity) && inclusiveFilterPassed && exclusiveFilterPassed)
				mGroupEntities.Add(entity);
		}

		void TryRemoveFromGroup(Entity entity)
		{
			mGroupEntities.EraseIfValid(entity);
		}

		std::array<SparseSet*, IncludedTypeCount> mIncludedTypeStorage { };
		std::array<SparseSet*, ExcludedTypeCount> mExcludedTypeStorage { };
		std::array<HEvent, (IncludedTypeCount + ExcludedTypeCount) * 2> mEventHandles;
		EntitySparseSet mGroupEntities;
	};



	/** @} */
} // namespace b3d::ecs
