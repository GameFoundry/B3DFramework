//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DUtility.h"
#include "ECS/B3DSparseSet.h"
#include "ECS/B3DEntityStorage.h"
#include "B3DUtility.h"
#include "Utility/B3DTypeId.h"

namespace b3d::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	template<typename IteratorType, typename OwnedStorageType, typename IncludedStorageType>
	struct TGroupIteratorAdapter {};

	/** Adapter around the provided iterator type that allows range for iteration. */
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
		constexpr TGroupIteratorAdapter(IteratorType iterator, std::tuple<OwnedStorageTypes*..., IncludedStorageTypes*...> ownedAndIncludedTypeStorage)
			: mIterator(iterator), mOwnedAndIncludedTypeStorage(std::move(ownedAndIncludedTypeStorage))
		{ }

		constexpr TGroupIteratorAdapter& operator++()
		{
			++mIterator;
			return *this;
		}

		/** Returns a tuple containing the entity and all components contained in owned and included storage types, respectively. */
		constexpr pointer operator->() const
		{
			return operator*();
		}

		/** Returns a tuple containing the entity and all components contained in owned and included storage types, respectively. */
		constexpr reference operator*() const
		{
			return std::tuple_cat(
				std::make_tuple(*mIterator),
				GetOwnedStorageElementTuple<OwnedStorageTypes>(*std::get<OwnedStorageTypes*>(mOwnedAndIncludedTypeStorage))...,
				GetAsTuple(std::get<IncludedStorageTypes*>(mOwnedAndIncludedTypeStorage), *mIterator)...);
		}

		/** Returns the underlying iterator that iterates over all entities in the group. */
		constexpr iterator_type GetUnderlyingIterator() const
		{
			return mIterator;
		}

		template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
		friend constexpr bool operator==(const TGroupIteratorAdapter<LeftIteratorTypes...>&, const TGroupIteratorAdapter<RightIteratorTypes...>&);

	private:
		/** Returns a tuple containing the component from the provided storage, at the current iterator location. */
		template<typename OwnedStorageType>
		auto GetOwnedStorageElementTuple(OwnedStorageType& storage) const
		{
			if constexpr(std::is_void_v<typename OwnedStorageType::ElementType>)
				return std::make_tuple();
			else
				return std::forward_as_tuple(storage.Begin()[mIterator.Index()]);
		}

		IteratorType mIterator;
		std::tuple<OwnedStorageTypes*..., IncludedStorageTypes*...> mOwnedAndIncludedTypeStorage;
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

	/** Common interface for all TGroup specializations. */
	struct GroupCommon
	{
		virtual ~GroupCommon() = default;

		virtual bool OwnsType(TypeHash elementTypeHash) const { return false; }
	};

	/**
	 * Provides helper functionality for owning groups.
	 *
	 * @tparam	OwnedTypeCount		Number of types owned by the group. All owned types will be sorted and tightly packed in their respective storage so they
	 *								may be quickly iterated over.
	 * @tparam	IncludedTypeCount	Number of types in the included type filter.
	 * @tparam	ExcludedTypeCount	Number of types in the excluded type filter.
	 */
	template<u32 OwnedTypeCount, u32 IncludedTypeCount, u32 ExcludedTypeCount>
	struct TGroupCommon : GroupCommon
	{
		template<typename... OwnedAndIncludedTypes, typename... ExcludedTypes>
		TGroupCommon(const std::tuple<OwnedAndIncludedTypes...>& ownedAndIncludedTypes, const std::tuple<ExcludedTypes...>& excludedTypes)
			: mIncludedTypeStorage(std::apply([](auto&&... storage) { return std::array<SparseSet*, OwnedTypeCount + IncludedTypeCount>({&storage...}); }, ownedAndIncludedTypes))
			, mExcludedTypeStorage(std::apply([](auto&&... storage) { return std::array<SparseSet*, ExcludedTypeCount>({&storage...}); }, excludedTypes))
		{
			u32 eventHandleIndex = 0;
			std::apply([this, &eventHandleIndex](auto&&... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage.OnWasAdded.Connect([this](Entity entity) { TryAddEntryToGroupAfterAdd(entity); }),
					mEventHandles[eventHandleIndex++] = storage.OnWillRemove.Connect([this](Entity entity) { TryRemoveFromGroup(entity); })), ...);
			}, ownedAndIncludedTypes);

			std::apply([this, &eventHandleIndex](auto&&... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage.OnWasAdded.Connect([this](Entity entity) { TryRemoveFromGroup(entity); }),
					mEventHandles[eventHandleIndex++] = storage.OnWillRemove.Connect([this](Entity entity) { TryAddEntryToGroupBeforeRemove(entity); })), ...);
			}, excludedTypes);
			
			for(auto entry : *mIncludedTypeStorage[0])
				TryAddEntryToGroupAfterAdd(entry);
		}

		~TGroupCommon()
		{
			for(auto& entry : mEventHandles)
				entry.Disconnect();
		}

		/** Returns the number of entities in the group. */
		u64 Size() const { return mNextIndex; }

		/** Returns the storage at the specified index. Storages are ordered starting with owned, then included and finally excluded storages. */
		template<u32 Index>
		SparseSet* GetStorage()
		{
			if constexpr(Index < (OwnedTypeCount + IncludedTypeCount))
				return mIncludedTypeStorage[Index];
			else
				return mExcludedTypeStorage[Index - (OwnedTypeCount + IncludedTypeCount)];
		}

		/** Checks if the provided type hash is an element type of one of the owned storage types. */
		bool OwnsType(TypeHash elementTypeHash) const override
		{
			for(u32 typeIndex = 0; typeIndex < OwnedTypeCount; ++typeIndex)
			{
				if(mIncludedTypeStorage[typeIndex]->GetElementTypeHash() == elementTypeHash)
					return true;
			}
			
			return false;
		}
		
	private:
		/**
		 * Swaps locations of two entities and their associated components in all owned storages.
		 *
		 * @param	packedIndex	Packed index of the entity to swap from.
		 * @param	entity		Entity to swap to.
		 */
		void SwapEntry(u64 packedIndex, Entity entity)
		{
			for(u32 storageIndex = 0; storageIndex < OwnedTypeCount; ++storageIndex)
				mIncludedTypeStorage[storageIndex]->Swap((*mIncludedTypeStorage[storageIndex])[packedIndex], entity);
		}

		/**
		 * Checks if the entity passes the group filter, and if so adds the entity to the packed entity list. This should be called whenever an entity is
		 * added to any referenced storage types, so we can check if the entity was added to the included type storage.
		 */
		void TryAddEntryToGroupAfterAdd(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity, index = mNextIndex](auto* firstStorage, auto*... otherStorage)
			{
				return firstStorage->Contains(entity) && firstStorage->GetPackedIndex(entity) >= index && (otherStorage->Contains(entity) && ...);
			}, mIncludedTypeStorage);

			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage) { return (!storage->Contains(entity) && ...); }, mExcludedTypeStorage);

			if(inclusiveFilterPassed && exclusiveFilterPassed)
				SwapEntry(mNextIndex++, entity);
		}

		/**
		 * Checks if the entity passes the group filter, and if so adds the entity to the packed entity list. This should be called whenever an entity is
		 * removed from any referenced storage types, so we can check if the entity has been removed from the last excluded storage.
		 */
		void TryAddEntryToGroupBeforeRemove(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity, index = mNextIndex](auto* firstStorage, auto*... otherStorage)
			{
				return firstStorage->Contains(entity) && firstStorage->GetPackedIndex(entity) >= index && (otherStorage->Contains(entity) && ...);
			}, mIncludedTypeStorage);

			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage)
			{
				return (0u + ... + storage->Contains(entity)) == 1u;
			}, mExcludedTypeStorage);

			if(inclusiveFilterPassed && exclusiveFilterPassed)
				SwapEntry(mNextIndex++, entity);
		}

		/** Removes an entity from the list of packed group entities. */
		void TryRemoveFromGroup(Entity entity)
		{
			if(mIncludedTypeStorage[0]->Contains(entity) && mIncludedTypeStorage[0]->GetPackedIndex(entity) < mNextIndex)
				SwapEntry(--mNextIndex, entity);
		}

		std::array<SparseSet*, OwnedTypeCount + IncludedTypeCount> mIncludedTypeStorage { };
		std::array<SparseSet*, ExcludedTypeCount> mExcludedTypeStorage { };
		std::array<HEvent, (OwnedTypeCount + IncludedTypeCount + ExcludedTypeCount) * 2> mEventHandles;
		u64 mNextIndex = 0;
	};

	/**
	 * Provides helper functionality for non-owning groups.
	 *
	 * @tparam	IncludedTypeCount	Number of types in the included type filter.
	 * @tparam	ExcludedTypeCount	Number of types in the excluded type filter.
	 */
	template<u32 IncludedTypeCount, u32 ExcludedTypeCount>
	struct TGroupCommon<0, IncludedTypeCount, ExcludedTypeCount> : GroupCommon
	{
		template<typename... IncludedTypes, typename... ExcludedTypes>
		TGroupCommon(const std::tuple<IncludedTypes...>& includedTypes, const std::tuple<ExcludedTypes...>& excludedTypes)
			: mIncludedTypeStorage(std::apply([](auto&&... storage) { return std::array<SparseSet*, IncludedTypeCount>({ &storage... }); }, includedTypes))
			, mExcludedTypeStorage(std::apply([](auto&&... storage) { return std::array<SparseSet*, ExcludedTypeCount>({ &storage... }); }, excludedTypes))
		{
			u32 eventHandleIndex = 0;
			std::apply([this, &eventHandleIndex](auto&&... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage.OnWasAdded.Connect([this](Entity entity) { TryAddEntryToGroupAfterAdd(entity); }),
					mEventHandles[eventHandleIndex++] = storage.OnWillRemove.Connect([this](Entity entity) { TryRemoveFromGroup(entity); })), ...);
			}, includedTypes);

			std::apply([this, &eventHandleIndex](auto&&... storage)
			{
				((mEventHandles[eventHandleIndex++] = storage.OnWasAdded.Connect([this](Entity entity) { TryRemoveFromGroup(entity); }),
					mEventHandles[eventHandleIndex++] = storage.OnWillRemove.Connect([this](Entity entity) { TryAddEntryToGroupBeforeRemove(entity); })), ...);
			}, excludedTypes);
			
			for(auto entry : *mIncludedTypeStorage[0])
				TryAddEntryToGroupAfterAdd(entry);
		}

		~TGroupCommon()
		{
			for(auto& entry : mEventHandles)
				entry.Disconnect();
		}

		/** Returns a set of entities that match the group filters. */
		TSparseSet<SparseSetDeletePolicy::SwapAndErase>& GetGroupStorage() { return mGroupEntities; }
		const TSparseSet<SparseSetDeletePolicy::SwapAndErase>& GetGroupStorage() const { return mGroupEntities; }

		/** Returns an included or excluded storage at a provided index. Included storage types are listed before excluded storage types. */
		template<u32 Index>
		SparseSet* GetStorage()
		{
			if constexpr(Index < IncludedTypeCount)
				return mIncludedTypeStorage[Index];
			else
				return mExcludedTypeStorage[Index - IncludedTypeCount];
		}
		
	private:
		/**
		 * Checks if the entity passes the group filter, and if so adds the entity to the packed entity list. This should be called whenever an entity is
		 * added to any referenced storage types, so we can check if the entity was added to the included type storage.
		 */
		void TryAddEntryToGroupAfterAdd(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity](auto*... storage) { return (storage->Contains(entity) && ...); }, mIncludedTypeStorage);
			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage) { return (!storage->Contains(entity) && ...); }, mExcludedTypeStorage);

			if(!mGroupEntities.Contains(entity) && inclusiveFilterPassed && exclusiveFilterPassed)
				mGroupEntities.Add(entity);
		}

		/**
		 * Checks if the entity passes the group filter, and if so adds the entity to the packed entity list. This should be called whenever an entity is
		 * removed from any referenced storage types, so we can check if the entity has been removed from the last excluded storage.
		 */
		void TryAddEntryToGroupBeforeRemove(Entity entity)
		{
			const bool inclusiveFilterPassed = std::apply([entity](auto*... storage) { return (storage->Contains(entity) && ...); }, mIncludedTypeStorage);
			const bool exclusiveFilterPassed = std::apply([entity](auto*... storage) { return (0u + ... + storage->Contains(entity)) == 1u; }, mExcludedTypeStorage);

			if(!mGroupEntities.Contains(entity) && inclusiveFilterPassed && exclusiveFilterPassed)
				mGroupEntities.Add(entity);
		}

		/** Removes an entity from the list of packed group entities. */
		void TryRemoveFromGroup(Entity entity)
		{
			mGroupEntities.EraseIfValid(entity);
		}

		std::array<SparseSet*, IncludedTypeCount> mIncludedTypeStorage { };
		std::array<SparseSet*, ExcludedTypeCount> mExcludedTypeStorage { };
		std::array<HEvent, (IncludedTypeCount + ExcludedTypeCount) * 2> mEventHandles;
		TSparseSet<SparseSetDeletePolicy::SwapAndErase> mGroupEntities;
	};

	template<typename, typename, typename>
	class TGroup;

	/**
	 * Non-owning group specialization. Acts similarly to a TView but ensures that entities are tightly packed for fast iteration,
	 * and allows sorting of entities.
	 *
	 * @tparam	IncludedStorageTypes	List of storage types that the entity must be a part of to be included in the group.
	 * @tparam	ExcludedStorageTypes	List of storage types that the entity must not be a part of to be included in the group.
	 */
	template<typename... IncludedStorageTypes, typename... ExcludedStorageTypes>
	class TGroup<TOwnedTypes<>, TIncludedTypes<IncludedStorageTypes...>, TExcludedTypes<ExcludedStorageTypes...>>
	{
		/** Returns storage type at the specified index. Included type storages are listed before excluded type storages. */
		template<u32 Index>
		using TStorageTypeAt = TTypeListElementAt<Index, TTypeList<IncludedStorageTypes..., ExcludedStorageTypes...>>;

		/** Returns the index of storage that contains the specified element type. Included type storages are listed before excluded type storages. */
		template<typename ElementType>
		static constexpr u32 TIndexOfElementType = TTypeListIndexOf<std::remove_const_t<ElementType>, TTypeList<typename IncludedStorageTypes::ElementType..., typename ExcludedStorageTypes::ElementType...>>;
		
	public:
		using Iterator = SparseSet::Iterator;
		using ReverseIterator = SparseSet::Iterator;
		using IteratorRange = TIteratorRange<TGroupIteratorAdapter<Iterator, TOwnedTypes<>, TIncludedTypes<IncludedStorageTypes...>>>;
		using GroupInternalsType = TGroupCommon<0, sizeof...(IncludedStorageTypes), sizeof...(ExcludedStorageTypes)>; 

		TGroup() = default;
		TGroup(GroupInternalsType& internals)
			:mInternals(&internals)
		{ }

		/**
		 * Returns a reference to the storage that is considered the leading storage. Leading storage is the storage that's primarily
		 * iterated over when looking for elements matching the group filter.
		 */
		const SparseSet& GetLeadingStorage() const
		{
			return mInternals->GetGroupStorage();
		}

		/** Returns a storage with the specified element type. Element type must be an element type of one of the included or excluded storage types. */
		template<typename ElementType>
		auto* GetStorage() const
		{
			return GetStorage<TIndexOfElementType<ElementType>>();
		}

		/** Returns a storage at the specified index. Included type storages are listed first, followed by excluded type storages. */
		template<u32 Index>
		auto* GetStorage() const
		{
			static_assert(Index < (sizeof...(IncludedStorageTypes) + sizeof...(ExcludedStorageTypes)), "Index out of range.");

			using StorageType = TStorageTypeAt<Index>;
			return static_cast<StorageType*>(mInternals != nullptr ? mInternals->template GetStorage<Index>() : nullptr);
		}

		/** Returns the number of entities that the group will iterate over. */
		u64 GetSize() const
		{
			return mInternals != nullptr ? GetLeadingStorage().Size() : 0;
		}

		/** Returns the reserved capacity of the group's leading storage. */
		u64 GetCapacity() const
		{
			return mInternals != nullptr ? GetLeadingStorage().Capacity() : 0;
		}

		/** Returns true if no entities pass the group filter. */
		bool IsEmpty() const
		{
			return mInternals != nullptr ? mInternals->IsEmpty() : true;
		}

		/** Reduces the leading storage capacity to match the current number of entities. */
		void Shrink()
		{
			if(mInternals != nullptr) GetLeadingStorage().Shrink();
		}

		/** Iterator to the first entity matching the group filter. */
		Iterator Begin() const { return mInternals != nullptr ? GetLeadingStorage().Begin() : Iterator{}; }

		/** Iterator past the last entity matching the group filter. */
		Iterator End() const { return mInternals != nullptr ? GetLeadingStorage().End() : Iterator{}; }

		/** Iterator to the last entity matching the group filter. */
		ReverseIterator Rbegin() const { return mInternals != nullptr ? GetLeadingStorage().Rbegin() : ReverseIterator{}; }

		/** Iterator before the first entity matching the group filter. */
		ReverseIterator Rend() const { return mInternals != nullptr ? GetLeadingStorage().Rend() : ReverseIterator{}; }

		/** Reference to the first entity matching the group filter, or null if none matches. */
		Entity Front() const
		{
			auto it = Begin();
			return it != End() ? *it : kNullEntity;
		}

		/** Reference to the last entity matching the group filter, or null if none matches. */
		Entity Back() const
		{
			auto it = Rbegin();
			return it != Rend() ? *it : kNullEntity;
		}

		/** Returns an iterator to the entity in the group, or End() iterator if no entity is found. */
		Iterator Find(Entity entity)
		{
			return mInternals != nullptr ? GetLeadingStorage().Find(entity) : Iterator{};
		}

		/** Returns true if the provided entity matches the group filter. */
		bool Contains(Entity entity)
		{
			return mInternals != nullptr ? GetLeadingStorage().Contains(entity) : false;
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. If no types are provided, all
		 * element types in the included group type filter are returned.
		 *
		 * @tparam	ElementType			Type of the first component to retrieve.
		 * @tparam	OtherElementType	Type of other components to retrieve, if any.
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing all the requested components, if multiple components are requested.
		 *								A single component if only one component is requested.
		 */
		template<typename ElementType, typename... OtherElementType>
		decltype(auto) Get(Entity entity) const
		{
			return Get<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementType>...>(entity);
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. If no indices are provided, all
		 * element types in the included group type filter are returned.
		 *
		 * @tparam	Indices				Indices referencing the included type filter whose components to retrieve.
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing all the requested components, if multiple components are requested.
		 *								A single component if only one component is requested.
		 */
		template<u32... Indices>
		decltype(auto) Get(Entity entity) const
		{
			const auto includedTypeStorage = GetIncludedStoragesAsTuple(std::index_sequence_for<IncludedStorageTypes...>{});

			if constexpr(sizeof...(Indices) == 0)
				return std::apply([entity](auto*... storage) { return std::tuple_cat(GetAsTuple(storage, entity)...); }, includedTypeStorage);
			else if constexpr(sizeof...(Indices) == 1)
				return (std::get<Indices>(includedTypeStorage)->Get(entity), ...);
			else
				return std::tuple_cat(GetAsTuple(std::get<Indices>(includedTypeStorage), entity)...);
		}

		/** Allows easy iteration over all components in the group using a range for loop. This is the fastest way to iterate all entries in a group. */
		IteratorRange Each() const
		{
			const auto includedTypeStorage = GetIncludedStoragesAsTuple(std::index_sequence_for<IncludedStorageTypes...>{});
			return IteratorRange({ Begin(), includedTypeStorage}, { End(), includedTypeStorage });
		}

		/**
		 * Calls @p function for each entry in the group. Valid signatures for @p function are:
		 *  (Entity, ComponentType&, ...) - Passes both entity and the component(s) to the function.
		 *  (ComponentType&, ...) - Passes only component(s) to the function.
		 */
		template<typename Function>
		void DoForEach(const Function& function)
		{
			for(const auto entity : *this)
			{
				// Check for (Entity, Type&, ...) signature
				if constexpr(TIsInvocableWithTupleArguments<Function, decltype(std::tuple_cat(std::tuple<Entity>{}, std::declval<TGroup>().Get({})))>)
					std::apply(function, std::tuple_cat(std::forward_as_tuple(entity), Get(entity)));
				else // Check for (Type&, ...) signature
					std::apply(function, Get(entity));
			}
		}

		/**
		 * Sorts the packed entity storage using the provided comparison function.
		 *
		 * @tparam	Indices				Indices of types in the included type filter, whose components to provide to the comparison function. If no indices are provided then
		 *								only entity will be provided to the comparison function.
		 * @tparam	ComparisonFunction	Function to use for comparison. Signature of the comparison function depends on the number of @p Indices template arguments:
		 *								 - 0 arguments: Comparison function accepts two Entity types
		 *								 - 1 argument:  Comparison function accepts two component types matching the element type of the storage at the provided index
		 *								 - >1 arguments:  Comparison function accepts two tuples that contain types matching the element type of the storage at the provided indices
		 */
		template <u32... Indices, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			if(mInternals == nullptr)
				return;

			if constexpr(sizeof...(Indices) == 0)
			{
				static_assert(std::is_invocable_v<ComparisonFunction, const Entity, const Entity>, "Invalid comparison function");
				mInternals->GetGroupStorage().Sort(std::move(predicate));
			}
			else
			{
				auto fnCompareElements = [&predicate, includedTypeStorage = GetIncludedStoragesAsTuple(std::index_sequence_for<IncludedStorageTypes...>{})](const Entity lhs, const Entity rhs)
				{
					if constexpr(sizeof...(Indices) == 1)
						return predicate((std::get<Indices>(includedTypeStorage)->Get(lhs), ...), (std::get<Indices>(includedTypeStorage)->Get(rhs), ...));
					else
						return predicate(std::forward_as_tuple(std::get<Indices>(includedTypeStorage)->Get(lhs)...), std::forward_as_tuple(std::get<Indices>(includedTypeStorage)->Get(rhs)...));
				};

				mInternals->GetGroupStorage().Sort(std::move(fnCompareElements));
			}
		}

		/**
		 * Sorts the packed entity storage using the provided comparison function.
		 *
		 * @tparam	ElementType				Type of the first component to provide to the comparison function.
		 * @tparam	OtherElementTypes		Type of other components to provide to the comparison function.
		 * @tparam	ComparisonFunction		Function to use for comparison. Signature of the comparison function depends on the number of provided type template arguments:
		 *									 - 0 arguments: Comparison function accepts two Entity types
		 *									 - 1 argument:  Comparison function accepts two component types matching the provided element type
		 *									 - >1 arguments:  Comparison function accepts two tuples that contain types matching the provided element types
		 */
		template <typename ElementType, typename... OtherElementTypes, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			Sort<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementTypes...>>(std::move(predicate));
		}

		/* Sorts the entity packed data to match the order of the provided entities. */
		template<typename It>
		void SortAs(It first, It last)
		{
			if(mInternals == nullptr)
				return;

			return mInternals->GetGroupStorage().SortAs(first, last);
		}

		Entity operator[](u64 index) const
		{
			return Begin()[index];
		}

		/** Returns type ID that uniquely identifies the type of this group. */
		static u64 TypeId()
		{
			return B3DGetTypeHash<TGroup<TOwnedTypes<>, TIncludedTypes<IncludedStorageTypes...>, TExcludedTypes<ExcludedStorageTypes...>>, u64>();
		}

		// For std compatibility
		using iterator = Iterator;
		using reverse_iterator = ReverseIterator;

		iterator begin() const { return Begin(); }
		iterator end() const { return End(); }
		reverse_iterator rbegin() const { return Rbegin(); }
		reverse_iterator rend() const { return Rend(); }

	private:
		/** Returns a tuple containing all included storages with the provided indices. */
		template<u32... Indices>
		auto GetIncludedStoragesAsTuple(std::index_sequence<Indices...>) const
		{
			using ReturnType = std::tuple<IncludedStorageTypes*...>;

			if(mInternals == nullptr)
				return ReturnType{};

			return ReturnType{static_cast<IncludedStorageTypes*>(mInternals->template GetStorage<Indices>())...};
		}

		GroupInternalsType* mInternals = nullptr;
	};

	/**
	 * Non-owning group specialization. Acts similarly to a TView but ensures that entities are tightly packed for fast iteration,
	 * and allows sorting of entities.
	 *
	 * @tparam	OwnedStorageTypes		List of storage types that the group will own. Owned storage types are guaranteed to pack their contents
	 *									in a way so that they can be quickly iterated over. Owned storage types can also be sorted.
	 * @tparam	IncludedStorageTypes	List of storage types that the entity must be a part of to be included in the group.
	 * @tparam	ExcludedStorageTypes	List of storage types that the entity must not be a part of to be included in the group.
	 */
	template<typename... OwnedStorageTypes, typename... IncludedStorageTypes, typename... ExcludedStorageTypes>
	class TGroup<TOwnedTypes<OwnedStorageTypes...>, TIncludedTypes<IncludedStorageTypes...>, TExcludedTypes<ExcludedStorageTypes...>>
	{
		static_assert(((OwnedStorageTypes::kDeletePolicy != SparseSetDeletePolicy::InPlace) && ...), "In-place delete not supported for owned storage.");

		/** Returns storage type at the specified index. Storages are listed in order: owned, included and then excluded. */
		template<u32 Index>
		using TStorageTypeAt = TTypeListElementAt<Index, TTypeList<OwnedStorageTypes..., IncludedStorageTypes..., ExcludedStorageTypes...>>;

		/** Returns the index of storage that contains the specified element type. Storages are listed in order: owned, included and then excluded. */
		template<typename ElementType>
		static constexpr u32 TIndexOfElementType = TTypeListIndexOf<std::remove_const_t<ElementType>, TTypeList<typename OwnedStorageTypes::ElementType..., typename IncludedStorageTypes::ElementType..., typename ExcludedStorageTypes::ElementType...>>;
		
	public:
		using Iterator = SparseSet::Iterator;
		using ReverseIterator = SparseSet::Iterator;
		using IteratorRange = TIteratorRange<TGroupIteratorAdapter<Iterator, TOwnedTypes<OwnedStorageTypes...>, TIncludedTypes<IncludedStorageTypes...>>>;
		using GroupInternalsType = TGroupCommon<sizeof...(OwnedStorageTypes), sizeof...(IncludedStorageTypes), sizeof...(ExcludedStorageTypes)>; 

		TGroup() = default;
		TGroup(GroupInternalsType& internals)
			:mInternals(&internals)
		{ }

		/**
		 * Returns a reference to the storage that is considered the leading storage. Leading storage is the owned storage that's primarily
		 * iterated over when looking for elements matching the group filter.
		 */
		const SparseSet& GetLeadingStorage() const
		{
			return *GetStorage<0>();
		}

		/** Returns a storage with the specified element type. Element type must be an element type of one of the owned, included or excluded storage types. */
		template<typename ElementType>
		auto* GetStorage() const
		{
			return GetStorage<TIndexOfElementType<ElementType>>();
		}

		/** Returns a storage at the specified index. Included type storages are listed first, followed by excluded type storages. */
		template<u32 Index>
		auto* GetStorage() const
		{
			static_assert(Index < (sizeof...(OwnedStorageTypes) + sizeof...(IncludedStorageTypes) + sizeof...(ExcludedStorageTypes)), "Index out of range.");

			using StorageType = TStorageTypeAt<Index>;
			return static_cast<StorageType*>(mInternals != nullptr ? mInternals->template GetStorage<Index>() : nullptr);
		}

		/** Returns the number of entities in the group. */
		u64 GetSize() const
		{
			return mInternals != nullptr ? mInternals->Size() : 0;
		}

		/** Returns true if there are no entities in the group or the group was not initialized. */
		bool IsEmpty() const
		{
			return mInternals != nullptr ? mInternals->Size() == 0 : true;
		}

		/** Iterator to the first entity matching the group filter. */
		Iterator Begin() const { return mInternals != nullptr ? GetLeadingStorage().Begin() : Iterator{}; }

		/** Iterator past the last entity matching the group filter. */
		Iterator End() const { return mInternals != nullptr ? GetLeadingStorage().Begin() + mInternals->Size() : Iterator{}; }

		/** Iterator to the last entity matching the group filter. */
		ReverseIterator Rbegin() const { return mInternals != nullptr ? GetLeadingStorage().Rbegin() + (GetLeadingStorage().Size() - mInternals->Size()) : ReverseIterator{}; }

		/** Iterator before the first entity matching the group filter. */
		ReverseIterator Rend() const { return mInternals != nullptr ? GetLeadingStorage().Rend() : ReverseIterator{}; }

		/** Reference to the first entity matching the group filter, or null if none matches. */
		Entity Front() const
		{
			auto it = Begin();
			return it != End() ? *it : kNullEntity;
		}

		/** Reference to the last entity matching the group filter, or null if none matches. */
		Entity Back() const
		{
			auto it = Rbegin();
			return it != Rend() ? *it : kNullEntity;
		}

		/** Returns an iterator to the entity in the group, or End() iterator if no entity is found. */
		Iterator Find(Entity entity)
		{
			auto found = mInternals != nullptr ? GetLeadingStorage().Find(entity) : Iterator{};
			return found <= End() ? found : Iterator();
		}

		/** Returns true if the provided entity matches the group filter. */
		bool Contains(Entity entity)
		{
			return mInternals != nullptr ? (GetLeadingStorage().Contains(entity) && GetLeadingStorage().GetPackedIndex(entity) < mInternals->Size()) : false;
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. If no types are provided, all
		 * element types in the included group type filter are returned.
		 *
		 * @tparam	ElementType			Type of the first component to retrieve.
		 * @tparam	OtherElementType	Type of other components to retrieve, if any.
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing all the requested components, if multiple components are requested.
		 *								A single component if only one component is requested.
		 */
		template<typename ElementType, typename... OtherElementType>
		decltype(auto) Get(Entity entity) const
		{
			return Get<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementType>...>(entity);
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. If no indices are provided, all
		 * element types in the included group type filter are returned.
		 *
		 * @tparam	Indices				Indices referencing the included type filter whose components to retrieve.
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing all the requested components, if multiple components are requested.
		 *								A single component if only one component is requested.
		 */
		template<u32... Indices>
		decltype(auto) Get(Entity entity) const
		{
			const auto ownedAndIncludedTypeStorage = GetOwnedAndIncludedStoragesAsTuple(std::index_sequence_for<OwnedStorageTypes...>{}, std::index_sequence_for<IncludedStorageTypes...>{});

			if constexpr(sizeof...(Indices) == 0)
				return std::apply([entity](auto*... storage) { return std::tuple_cat(GetAsTuple(storage, entity)...); }, ownedAndIncludedTypeStorage);
			else if constexpr(sizeof...(Indices) == 1)
				return (std::get<Indices>(ownedAndIncludedTypeStorage)->Get(entity), ...);
			else
				return std::tuple_cat(GetAsTuple(std::get<Indices>(ownedAndIncludedTypeStorage), entity)...);
		}

		/** Allows easy iteration over all components in the group using a range for loop. This is the fastest way to iterate all entries in a group. */
		IteratorRange Each() const
		{
			const auto ownedAndIncludedTypeStorage = GetOwnedAndIncludedStoragesAsTuple(std::index_sequence_for<OwnedStorageTypes...>{}, std::index_sequence_for<IncludedStorageTypes...>{});
			return IteratorRange({ Begin(), ownedAndIncludedTypeStorage}, { End(), ownedAndIncludedTypeStorage });
		}

		/**
		 * Calls @p function for each entry in the group. Valid signatures for @p function are:
		 *  (Entity, ComponentType&, ...) - Passes both entity and the component(s) to the function.
		 *  (ComponentType&, ...) - Passes only component(s) to the function.
		 */
		template<typename Function>
		void DoForEach(const Function& function)
		{
			for(auto tuple : Each())
			{
				// Check for (Entity, Type&, ...) signature
				if constexpr(TIsInvocableWithTupleArguments<Function, decltype(std::tuple_cat(std::tuple<Entity>{}, std::declval<TGroup>().Get({})))>)
					std::apply(function, tuple);
				else // Check for (Type&, ...) signature (fast path)
					std::apply([&function](auto, auto&&... otherElements) { function(std::forward<decltype(otherElements)>(otherElements)...); }, tuple);
			}
		}

		/**
		 * Sorts the packed entity storage using the provided comparison function.
		 *
		 * @tparam	Indices				Indices of types in the included type filter, whose components to provide to the comparison function. If no indices are provided then
		 *								only entity will be provided to the comparison function.
		 * @tparam	ComparisonFunction	Function to use for comparison. Signature of the comparison function depends on the number of @p Indices template arguments:
		 *								 - 0 arguments: Comparison function accepts two Entity types
		 *								 - 1 argument:  Comparison function accepts two component types matching the element type of the storage at the provided index
		 *								 - >1 arguments:  Comparison function accepts two tuples that contain types matching the element type of the storage at the provided indices
		 */
		template <u32... Indices, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			if(mInternals == nullptr)
				return;

			const auto ownedAndIncludedTypeStorage = GetOwnedAndIncludedStoragesAsTuple(std::index_sequence_for<OwnedStorageTypes...>{}, std::index_sequence_for<IncludedStorageTypes...>{});
			if constexpr(sizeof...(Indices) == 0)
			{
				static_assert(std::is_invocable_v<ComparisonFunction, const Entity, const Entity>, "Invalid comparison function");
				GetStorage<0>()->SortN(mInternals->Size(), std::move(predicate));
			}
			else
			{
				auto fnCompareElements = [&predicate, &ownedAndIncludedTypeStorage](const Entity lhs, const Entity rhs)
				{
					if constexpr(sizeof...(Indices) == 1)
						return predicate((std::get<Indices>(ownedAndIncludedTypeStorage)->Get(lhs), ...), (std::get<Indices>(ownedAndIncludedTypeStorage)->Get(rhs), ...));
					else
						return predicate(std::forward_as_tuple(std::get<Indices>(ownedAndIncludedTypeStorage)->Get(lhs)...), std::forward_as_tuple(std::get<Indices>(ownedAndIncludedTypeStorage)->Get(rhs)...));
				};

				GetStorage<0>()->SortN(mInternals->Size(), std::move(fnCompareElements));
			}

			// Sort all other owned storages in the same order as leading storage
			auto fnSortOtherStorages = [this](auto* leadingStorage, auto*... otherStorages)
			{
				for(u64 nextIndex = mInternals->Size(); nextIndex > 0; --nextIndex)
				{
					const u64 entryIndex = nextIndex - 1;
					const Entity entity = leadingStorage->Data()[entryIndex];
					(otherStorages->Swap(otherStorages->Data()[entryIndex], entity), ...);
				}
			};

			std::apply(fnSortOtherStorages, ownedAndIncludedTypeStorage);
		}

		/**
		 * Sorts the packed entity storage using the provided comparison function.
		 *
		 * @tparam	ElementType				Type of the first component to provide to the comparison function.
		 * @tparam	OtherElementTypes		Type of other components to provide to the comparison function.
		 * @tparam	ComparisonFunction		Function to use for comparison. Signature of the comparison function depends on the number of provided type template arguments:
		 *									 - 0 arguments: Comparison function accepts two Entity types
		 *									 - 1 argument:  Comparison function accepts two component types matching the provided element type
		 *									 - >1 arguments:  Comparison function accepts two tuples that contain types matching the provided element types
		 */
		template <typename ElementType, typename... OtherElementTypes, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			Sort<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementTypes...>>(std::move(predicate));
		}

		Entity operator[](u64 index) const
		{
			return Begin()[index];
		}

		/** Returns type ID that uniquely identifies the type of this group. */
		static u64 TypeId()
		{
			return B3DGetTypeHash<TGroup<TOwnedTypes<OwnedStorageTypes...>, TIncludedTypes<IncludedStorageTypes...>, TExcludedTypes<ExcludedStorageTypes...>>, u64>();
		}

		// For std compatibility
		using iterator = Iterator;
		using reverse_iterator = ReverseIterator;

		iterator begin() const { return Begin(); }
		iterator end() const { return End(); }
		iterator rbegin() const { return Rbegin(); }
		iterator rend() const { return Rend(); }

	private:
		/** Returns a tuple containing all owned and included storages with the provided indices. */
		template<u32... OwnedIndices, u32... IncludedIndices>
		auto GetOwnedAndIncludedStoragesAsTuple(std::index_sequence<OwnedIndices...>, std::index_sequence<IncludedIndices...>) const
		{
			using ReturnType = std::tuple<OwnedStorageTypes*..., IncludedStorageTypes*...>;

			if(mInternals == nullptr)
				return ReturnType{};

			return ReturnType{static_cast<OwnedStorageTypes*>(mInternals->template GetStorage<OwnedIndices>())..., static_cast<IncludedStorageTypes*>(mInternals->template GetStorage<sizeof...(OwnedStorageTypes) + IncludedIndices>())...};
		}

		GroupInternalsType* mInternals = nullptr;
	};

	/** @} */
} // namespace b3d::ecs
