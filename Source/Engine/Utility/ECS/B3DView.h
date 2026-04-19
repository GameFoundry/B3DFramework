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
	/** @addtogroup ECS-Internal
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	/** Checks if the entity is part of all storages in the provided range. */
	template<typename It>
	static bool IsEntityPartOfAll(It first, It last, Entity entity)
	{
		while(first != last && (*first)->Contains(entity))
			++first;

		return first == last;
	}

	/** Checks if the entity is part of any storages in the provided range. */
	template<typename It>
	static bool IsEntityPartOfAny(It first, It last, Entity entity)
	{
		while(first != last && !(*first)->Contains(entity))
			++first;

		return first != last;
	}

	/**
	 * Checks if entity matches the included & excluded type filters.
	 *
	 * @tparam	IncludedTypeCount						Number of types in the included type filter.
	 * @tparam	ExcludedTypeCount						Number of types in the excluded type filter.
	 * @tparam	StorageMayContainInvalidEntities		True if the storage may contain invalid entities (i.e. storage is using in-place delete).
	 *
	 * @param	entity									Entity to check.
	 * @param	includedTypeStorage						Storages that must contain the entity to pass the filter.
	 * @param	excludedTypeStorage						Storages that must not contain the entity to pass the filter.
	 * @param	leadingTypeIndex						Index of the included type storage which is leading, and therefore guaranteed to contain the entity.
	 * @return
	 */
	template<u32 IncludedTypeCount, u32 ExcludedTypeCount, bool StorageMayContainInvalidEntities>
	static bool DoesEntityMatchFilter(Entity entity, const std::array<const SparseSet*, IncludedTypeCount>& includedTypeStorage, const std::array<const SparseSet*, ExcludedTypeCount>& excludedTypeStorage, u32 leadingTypeIndex)
	{
		// Must not be deleted entity (only relevant for in-place deletion), must be contained in all the included type storages, and must not be part of any excluded type storages
		return (!StorageMayContainInvalidEntities || entity != kInvalidEntity)
			&& ((IncludedTypeCount == 1u) || (IsEntityPartOfAll(includedTypeStorage.begin(), includedTypeStorage.begin() + leadingTypeIndex, entity) && IsEntityPartOfAll(includedTypeStorage.begin() + leadingTypeIndex + 1, includedTypeStorage.end(), entity)))
			&& ((ExcludedTypeCount == 0u) || (!IsEntityPartOfAny(excludedTypeStorage.begin(), excludedTypeStorage.end(), entity)));
	}

	/**
	 * Iterator that iterates over all entities that match the view included & excluded type filter.
	 *
	 * @tparam	IncludedTypeCount	Number of types in the included type filter.
	 * @tparam	ExcludedTypeCount	Number of types in the excluded type filter.
	 * @tparam	InPlaceDelete		True if the storages support in-place delete policy.
	 */
	template<u32 IncludedTypeCount, u32 ExcludedTypeCount, bool InPlaceDelete>
	struct TViewIterator final
	{
	private:
		using UnderlyingIterator = SparseSet::ConstIterator;

	public:
		using value_type = std::iterator_traits<UnderlyingIterator>::value_type;
		using pointer = std::iterator_traits<UnderlyingIterator>::pointer;
		using reference = std::iterator_traits<UnderlyingIterator>::reference;
		using difference_type = std::iterator_traits<UnderlyingIterator>::difference_type;
		using iterator_category = std::forward_iterator_tag;

		TViewIterator() = default;
		TViewIterator(UnderlyingIterator underlyingIterator, std::array<const SparseSet*, IncludedTypeCount> includedTypeStorage, std::array<const SparseSet*, ExcludedTypeCount> excludedTypeStorage, u32 leadingTypeIndex)
			: mUnderlyingIterator(underlyingIterator), mIncludedTypeStorage(std::move(includedTypeStorage)), mExcludedTypeStorage(std::move(excludedTypeStorage)), mLeadingTypeIndex(leadingTypeIndex)
		{
			SeekToNextValidEntry();
		}

		TViewIterator& operator++()
		{
			++mUnderlyingIterator;
			SeekToNextValidEntry();

			return *this;
		}

		pointer operator->() const
		{
			return &*mUnderlyingIterator;
		}

		reference operator*() const
		{
			return *mUnderlyingIterator;
		}

		std::array<const SparseSet*, IncludedTypeCount>& GetIncludedTypeStorage() { return mIncludedTypeStorage; }
		const std::array<const SparseSet*, IncludedTypeCount>& GetIncludedTypeStorage() const { return mIncludedTypeStorage; }

		template<u32 IncludedTypeCount2, u32 ExcludedTypeCount2, bool InPlaceDelete2>
		friend constexpr bool operator==(const TViewIterator<IncludedTypeCount2, ExcludedTypeCount2, InPlaceDelete2>& lhs, const TViewIterator<IncludedTypeCount2, ExcludedTypeCount2, InPlaceDelete2>& rhs) noexcept;

		template<u32 IncludedTypeCount2, u32 ExcludedTypeCount2, bool InPlaceDelete2>
		friend constexpr bool operator!=(const TViewIterator<IncludedTypeCount2, ExcludedTypeCount2, InPlaceDelete2>& lhs, const TViewIterator<IncludedTypeCount2, ExcludedTypeCount2, InPlaceDelete2>& rhs) noexcept;

	private:
		/**
		 * Increments the leading type storage to the next entry and check if it matches the type filter. If it doesn't match the type
		 * filter the next entry is searched, and so on until the first matching entry or the end of leading type storage.
		 */
		void SeekToNextValidEntry()
		{
			// Iterate until the find next matching entity
			while(mUnderlyingIterator != mIncludedTypeStorage[mLeadingTypeIndex]->End() && !DoesEntityMatchFilter<IncludedTypeCount, ExcludedTypeCount, InPlaceDelete>(*mUnderlyingIterator, mIncludedTypeStorage, mExcludedTypeStorage, mLeadingTypeIndex))
				++mUnderlyingIterator;
		}

		UnderlyingIterator mUnderlyingIterator;
		std::array<const SparseSet*, IncludedTypeCount> mIncludedTypeStorage;
		std::array<const SparseSet*, ExcludedTypeCount> mExcludedTypeStorage;
		u32 mLeadingTypeIndex = 0;
	};

	template<u32 IncludedTypeCount, u32 ExcludedTypeCount, bool InPlaceDelete>
	constexpr bool operator==(const TViewIterator<IncludedTypeCount, ExcludedTypeCount, InPlaceDelete>& lhs, const TViewIterator<IncludedTypeCount, ExcludedTypeCount, InPlaceDelete>& rhs) noexcept
	{
		return lhs.mUnderlyingIterator == rhs.mUnderlyingIterator;
	}

	template<u32 IncludedTypeCount, u32 ExcludedTypeCount, bool InPlaceDelete>
	constexpr bool operator!=(const TViewIterator<IncludedTypeCount, ExcludedTypeCount, InPlaceDelete>& lhs, const TViewIterator<IncludedTypeCount, ExcludedTypeCount, InPlaceDelete>& rhs) noexcept
	{
		return !(lhs.mUnderlyingIterator == rhs.mUnderlyingIterator);
	}

	/** Adapter around TViewIterator that allows range for iteration. */
	template<typename IteratorType, typename... IncludedStorageType>
	struct TViewIteratorAdapter
	{
	public:
		using iterator_type = IteratorType;
		using value_type = decltype(std::tuple_cat(std::make_tuple(*std::declval<IteratorType>()), GetAsTuple<IncludedStorageType>(nullptr, {})...));
		using pointer = TPointerToTemporary<value_type>;
		using reference = value_type;
		using difference_type = typename std::iterator_traits<IteratorType>::difference_type;
		using iterator_category = std::input_iterator_tag;

		constexpr TViewIteratorAdapter() = default;
		constexpr TViewIteratorAdapter(IteratorType iterator)
			: mIterator(iterator)
		{ }

		constexpr TViewIteratorAdapter& operator++()
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
			return GetTuple(std::index_sequence_for<IncludedStorageType...>());
		}

		constexpr iterator_type GetEntityIterator() const
		{
			return mIterator;
		}

		template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
		friend constexpr bool operator==(const TViewIteratorAdapter<LeftIteratorTypes...>&, const TViewIteratorAdapter<RightIteratorTypes...>&);

	private:
		/** Returns a tuple containing components from included storage at the provided indices. First entry in the tuple is the entity the components are associated with. */
		template<size_t... Index>
		reference GetTuple(std::index_sequence<Index...>) const
		{
			return std::tuple_cat(std::make_tuple(*mIterator), GetAsTuple(static_cast<IncludedStorageType *>(std::get<Index>(mIterator.GetIncludedTypeStorage())), *mIterator)...);
		}

		IteratorType mIterator;
	};

	template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
	constexpr bool operator==(const TViewIteratorAdapter<LeftIteratorTypes...>& lhs, const TViewIteratorAdapter<RightIteratorTypes...>& rhs)
	{
		return lhs.GetEntityIterator() == rhs.GetEntityIterator();
	}

	template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
	constexpr bool operator!=(const TViewIteratorAdapter<LeftIteratorTypes...>& lhs, const TViewIteratorAdapter<RightIteratorTypes...>& rhs)
	{
		return !(lhs == rhs);
	}

	/**
	 * Provides helper functionality for a view containing multiple (more than one) included or excluded types.
	 *
	 * @tparam	IncludedTypeCount	Number of types in the included type filter.
	 * @tparam	ExcludedTypeCount	Number of types in the excluded type filter.
	 * @tparam	InPlaceDelete		True if the storages support in-place delete policy.
	 */
	template<u32 IncludedTypeCount, u32 ExcludedTypeCount, bool InPlaceDelete>
	class TMultiStorageViewCommon
	{
	public:
		using Iterator = TViewIterator<IncludedTypeCount, ExcludedTypeCount, InPlaceDelete>;

		/**
		 * Returns pointer to the storage that is considered the leading storage. Leading storage is the storage that's primarily
		 * iterated over when looking for elements matching the view filter.
		 */
		const SparseSet* GetLeadingTypeStorage() const
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return nullptr;

			return mIncludedTypeStorage[mLeadingTypeIndex];
		}

		/** Returns a rough number of elements in the view, based on the number of elements in the leading storage. */
		u64 GetSizeEstimate() const
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return 0u;

			return GetLeadingTypeStorageSize();
		}

		/** Iterator to the first entity matching the view filter. */
		Iterator Begin() const
		{
			return mLeadingTypeIndex != IncludedTypeCount ? Iterator(mIncludedTypeStorage[mLeadingTypeIndex]->Begin(), mIncludedTypeStorage, mExcludedTypeStorage, mLeadingTypeIndex) : Iterator();
		}

		/** Iterator past the last entity matching the view filter. */
		Iterator End() const
		{
			return mLeadingTypeIndex != IncludedTypeCount ? Iterator(mIncludedTypeStorage[mLeadingTypeIndex]->Begin() + GetLeadingTypeStorageSize(), mIncludedTypeStorage, mExcludedTypeStorage, mLeadingTypeIndex) : Iterator();
		}

		/** Reference to the first entity matching the view filter, or null if none matches. */
		Entity Front() const
		{
			auto it = Begin();
			return  it != End() ? *it : kNullEntity;
		}

		/** Reference to the last entity matching the view filter, or null if none matches. */
		Entity Back() const
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return kNullEntity;

			auto it = mIncludedTypeStorage[mLeadingTypeIndex]->Rbegin();
			const auto last = it + GetLeadingTypeStorageSize();

			while (it != last && !DoesEntityMatchViewFilter(*it))
				++it;

			return it != last ? *it : kNullEntity;
		}

		/** Returns an iterator to the entity in the view, or End() iterator if no entity is found. */
		Iterator Find(Entity entity)
		{
			if(Contains(entity))
				return Iterator(mIncludedTypeStorage[mLeadingTypeIndex]->Find(entity), mIncludedTypeStorage, mExcludedTypeStorage, mLeadingTypeIndex);

			return End();
		}

		/** Returns true if the provided entity matches the view filter. */
		bool Contains(Entity entity)
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return false;

			return IsEntityPartOfAll(mIncludedTypeStorage.begin(), mIncludedTypeStorage.end(), entity)
				&& !IsEntityPartOfAny(mExcludedTypeStorage.begin(), mExcludedTypeStorage.end(), entity)
				&& (mIncludedTypeStorage[mLeadingTypeIndex]->GetPackedIndex(entity) < GetLeadingTypeStorageSize());
		}

		/** Returns true if all included and excluded storage pointer are initializes. View is not usable unless this returns true. */
		bool IsValid()
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return false;

			for(const auto& entry : mExcludedTypeStorage)
			{
				if(entry == GetPlaceholderStorage())
					return false;
			}

			return true;
		}

		// For std compatibility
		using iterator = Iterator;

		iterator begin() { return Begin(); }
		iterator end() { return End(); }

	protected:
		TMultiStorageViewCommon()
		{
			for(auto& entry : mExcludedTypeStorage)
				entry = GetPlaceholderStorage();
		}

		TMultiStorageViewCommon(std::array<const SparseSet*, IncludedTypeCount>& includedTypeStorage, std::array<const SparseSet*, ExcludedTypeCount> excludedTypeStorage)
			:mIncludedTypeStorage(includedTypeStorage), mExcludedTypeStorage(excludedTypeStorage), mLeadingTypeIndex(IncludedTypeCount)
		{
			RefreshLeadingTypeIndex();
		}

		/** Scans the included type storage list for smallest storage, and marks it as the leading storage. */
		void RefreshLeadingTypeIndex()
		{
			mLeadingTypeIndex = 0;

			if constexpr(IncludedTypeCount > 0)
			{
				for(u32 typeIndex = 1; typeIndex < IncludedTypeCount; ++typeIndex)
				{
					if(mIncludedTypeStorage[typeIndex]->Size() < mIncludedTypeStorage[mLeadingTypeIndex]->Size())
						mLeadingTypeIndex = typeIndex;
				}
			}
		}

		/** Refreshes the leading type index, but only if all included type storage pointers were assigned. */
		void RefreshLeadingTypeIndexIfNeeded()
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
			{
				u32 count = 0;
				for(; count < IncludedTypeCount; ++count)
				{
					if(mIncludedTypeStorage[count] == nullptr)
						break;
				}

				// If all storage types are assigned
				if(count == IncludedTypeCount)
					RefreshLeadingTypeIndex();
			}
			else
				RefreshLeadingTypeIndex();
		}

		/** Returns the included type storage pointer at the provided index. */
		const SparseSet* GetIncludedTypeStorage(u32 index) const
		{
			return mIncludedTypeStorage[index];
		}

		/** Assigns the included type storage pointer at the provided index. You must assign all storages before using the view. */
		void SetIncludedTypeStorage(u32 index, const SparseSet* storage)
		{
			B3D_ASSERT(storage != nullptr);

			mIncludedTypeStorage[index] = storage;
			RefreshLeadingTypeIndexIfNeeded();
		}

		/** Returns the excluded type storage pointer at the provided index. */
		const SparseSet* GetExcludedTypeStorage(u32 index) const
		{
			return mExcludedTypeStorage[index] == GetPlaceholderStorage() ? nullptr : mExcludedTypeStorage[index];
		}

		/** Assigns the excluded type storage pointer at the provided index. If excluded type storage is unassigned, it will be ignored by the view filter. */
		void SetExcludedTypeStorage(u32 index, const SparseSet* storage)
		{
			B3D_ASSERT(storage != nullptr);

			mExcludedTypeStorage[index] = storage;
		}

		/** Sets leading type index explicitly to the specific included type index. By default the smallest storage is used. */
		void SetExplicitLeadingTypeIndex(u32 index)
		{
			// Must assign all storage types before doing this, as refreshing type index during storage
			// assignment relies on mLeadingTypeIndex == IncludedTypeCount until all storage is assigned
			B3D_ASSERT(mLeadingTypeIndex != IncludedTypeCount);

			mLeadingTypeIndex = index;
		}

		/** Returns the number of elements in the leading storage. If leading storage uses in-place deletion policy, this number will also contain invalid elements. */
		u64 GetLeadingTypeStorageSize() const
		{
			B3D_ASSERT(mLeadingTypeIndex != IncludedTypeCount);
			return mIncludedTypeStorage[mLeadingTypeIndex]->GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly ? mIncludedTypeStorage[mLeadingTypeIndex]->GetFirstFreeElementPackedIndex() : mIncludedTypeStorage[mLeadingTypeIndex]->Size();
		}

		/** Returns empty storage that can be used for excluded type filter when no storage is assigned yet. */
		static const SparseSet* GetPlaceholderStorage()
		{
			static const EntitySparseSet kPlaceholder;
			return &kPlaceholder;
		}

		std::array<const SparseSet*, IncludedTypeCount> mIncludedTypeStorage { };
		std::array<const SparseSet*, ExcludedTypeCount> mExcludedTypeStorage { };
		u32 mLeadingTypeIndex = IncludedTypeCount;
	};

	/** @} */

	/** @addtogroup ECS
	 *  @{
	 */

	template<typename, typename, typename = void>
	class TView;

	/**
	 * Creates a view that allows you to iterate over all entities that match included & excluded type filter.
	 *
	 * @tparam	IncludedStorageType	List of storage types that the entity must be a part of to be included in the view.
	 * @tparam	ExcludedStorageType	List of storage types that the entity must not be a part of to be included in the view.
	 */
	template<typename... IncludedStorageType, typename... ExcludedStorageType>
	class TView<TIncludedTypes<IncludedStorageType...>, TExcludedTypes<ExcludedStorageType...>, std::enable_if_t<(sizeof...(IncludedStorageType) != 0u)>> : public TMultiStorageViewCommon<sizeof...(IncludedStorageType), sizeof...(ExcludedStorageType), TAllTypesUseInPlaceDelete<IncludedStorageType...>>
	{
		using Super = TMultiStorageViewCommon<sizeof...(IncludedStorageType), sizeof...(ExcludedStorageType), TAllTypesUseInPlaceDelete<IncludedStorageType...>>;

		/** Returns storage type at the specified index. Included type storages are listed before excluded type storages. */
		template<u32 Index>
		using TStorageTypeAt = TTypeListElementAt<Index, TTypeList<IncludedStorageType..., ExcludedStorageType...>>;

		/** Returns the index of storage that contains the specified element type. Included type storages are listed before excluded type storages. */
		template<typename ElementType>
		static constexpr u32 TIndexOfElementType = TTypeListIndexOf<std::remove_const_t<ElementType>, TTypeList<typename IncludedStorageType::ElementType..., typename ExcludedStorageType::ElementType...>>;
		
	public:
		using IteratorRange = TIteratorRange<TViewIteratorAdapter<typename Super::Iterator, IncludedStorageType...>>;

		TView() = default;
		TView(IncludedStorageType... includedType, ExcludedStorageType... excludedType)
			:Super::Super({includedType...}, {excludedType...})
		{ }

		TView(std::tuple<IncludedStorageType&...> includedTypes, std::tuple<ExcludedStorageType&...> excludedTypes = {})
			:TView(std::make_from_tuple<TView>(std::tuple_cat(includedTypes, excludedTypes)))
		{ }

		/**
		 * Sets an explicit leading type to iterate over. Leading storage type is the primary storage that will be iterated over by the view.
		 * By default this will be set automatically to the smallest storage (one with least elements).
		 *
		 * @tparam ElementType		Element type of the storage to set as the leading type. Must be an element type of one of the included storage types.
		 */
		template<typename ElementType>
		void SetLeadingType()
		{
			SetLeadingType<TIndexOfElementType<ElementType>>();
		}

		/**
		 * Sets an explicit leading type to iterate over. Leading storage type is the primary storage that will be iterated over by the view.
		 * By default this will be set automatically to the smallest storage (one with least elements).
		 *
		 * @tparam Index		Index of the storage to set as the leading type, in the included storage type list.
		 */
		template<u32 Index>
		void SetLeadingType()
		{
			Super::SetExplicitLeadingTypeIndex(Index);
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
			static_assert(Index < (sizeof...(IncludedStorageType) + sizeof...(ExcludedStorageType)), "Index out of range.");

			if constexpr(Index < sizeof...(IncludedStorageType))
				return static_cast<TStorageTypeAt<Index>*>(const_cast<TInheritConstFrom<SparseSet, TStorageTypeAt<Index>>*>(Super::GetIncludedTypeStorage(Index)));
			else
				return static_cast<TStorageTypeAt<Index>*>(const_cast<TInheritConstFrom<SparseSet, TStorageTypeAt<Index>>*>(Super::GetExcludedTypeStorage(Index - sizeof...(IncludedStorageType))));
		}

		/** Assigns a storage to the view. All included & excluded storage types should be assigned before using the view. */
		template<typename StorageType>
		void SetStorage(StorageType& storage)
		{
			SetStorage<TIndexOfElementType<typename StorageType::ElementType>>(storage);
		}

		decltype(auto) operator[](Entity entity) const
		{
			return Get(entity);
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. If no types are provided, all
		 * element types in the included view type filter are returned.
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
		 * element types in the included view type filter are returned.
		 *
		 * @tparam	Index				Indices referencing the included type filter whose components to retrieve.
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing all the requested components, if multiple components are requested.
		 *								A single component if only one component is requested.
		 */
		template<u32... Index>
		decltype(auto) Get(Entity entity) const
		{
			if constexpr(sizeof...(Index) == 0)
				return std::tuple_cat(GetAsTuple(GetStorage<IncludedStorageType::ElementType>(), entity)...);
			else if constexpr(sizeof...(Index) == 1)
				return (GetStorage<Index>()->Get(entity), ...);
			else
				return std::tuple_cat(GetAsTuple(GetStorage<Index>(), entity)...);
		}

		/** Allows easy iteration over all components in the view using a range for loop. */
		IteratorRange Each() const
		{
			return IteratorRange(Super::Begin(), Super::End());
		}

		/**
		 * Calls @p function for each entry in the view. Valid signatures for @p function are:
		 *  (Entity, ComponentType&, ...) - Passes both entity and the component(s) to the function.
		 *  (ComponentType&, ...) - Passes only component(s) to the function.
		 */
		template<typename Function>
		void DoForEach(Function&& function)
		{
			FindLeadingStorageIndexAndCallFunctionForEachEntity(std::forward<Function>(function), std::index_sequence_for<IncludedStorageType...>());
		}

	private:
		/** Helper that assigns the storage at the specified index. */
		template<u32 Index, typename StorageType>
		void SetStorage(StorageType& storage)
		{
			static_assert(Index < (sizeof...(IncludedStorageType) + sizeof...(ExcludedStorageType)), "Index out of range.");
			static_assert(std::is_convertible_v<StorageType, TStorageTypeAt<Index>>, "Unsupported type.");

			if constexpr(Index < sizeof...(IncludedStorageType))
				Super::SetIncludedTypeStorage(Index, &storage);
			else
				Super::SetExcludedTypeStorage(Index - sizeof...(IncludedStorageType), &storage);
		}

		/**
		 * Returns @p leadingEntry argument if LeadingTypeindex == OtherTypeIndex, otherwise uses the Entity
		 * from @p leadingEntry to look up the entry in the relevant storage for @p OtherTypeIndex.
		 */
		template<u32 LeadingTypeIndex, u32 OtherTypeIndex, typename... Arguments>
		auto GetElementFromStorageOrArgument(const std::tuple<Entity, Arguments...>& leadingEntry)
		{
			if constexpr(LeadingTypeIndex == OtherTypeIndex)
				return std::forward_as_tuple(std::get<Arguments>(leadingEntry)...);
			else
				return GetAsTuple(GetStorage<OtherTypeIndex>(), std::get<0>(leadingEntry));
		}

		/** Calls @p function for each entry contained in the storage at @p LeadingTypeIndex. */
		template<u32 LeadingTypeIndex, typename Function, size_t... TypeIndex>
		void CallFunctionForEachEntity(const Function& function, std::index_sequence<TypeIndex...>)
		{
			for(const auto entry : GetStorage<LeadingTypeIndex>()->Each())
			{
				const auto entity = std::get<0>(entry);
				if((!TAllTypesUseInPlaceDelete<IncludedStorageType...> || entity != kInvalidEntity) // Check if invalid entities, if using in-place delete
					&& ((LeadingTypeIndex == TypeIndex || Super::GetIncludedTypeStorage(TypeIndex)->Contains(entity)) && ...) // Ensure all the included storages contain the entity (for leading type we just check the index as its guaranteed)
					&& !IsEntityPartOfAny(this->mExcludedTypeStorage.begin(), this->mExcludedTypeStorage.end(), entity)) // Ensure none of the excluded storages contain the entity
				{
					// Check for (Entity, Type&, ...) signature
					if constexpr(TIsInvocableWithTupleArguments<Function, decltype(std::tuple_cat(std::tuple<Entity>{}, std::declval<TView>().Get({})))>)
						std::apply(function, std::tuple_cat(std::forward_as_tuple(entity), GetElementFromStorageOrArgument<LeadingTypeIndex, TypeIndex>(entry)...));
					else // Check for (Type&, ...) signature
					{
						static_assert(TIsInvocableWithTupleArguments<Function, decltype(std::declval<TView>().Get({}))>, "Invalid function signature.");
						std::apply(function, std::tuple_cat(GetElementFromStorageOrArgument<LeadingTypeIndex, TypeIndex>(entry)...));
					}
				}
			}
		}

		/** Find the index of leading type storage and calls the appropriate instantiation of CallFunctionForEachEntity. */
		template<typename Function, size_t... TypeIndex>
		void FindLeadingStorageIndexAndCallFunctionForEachEntity(const Function& function, std::index_sequence<TypeIndex...> sequence)
		{
			auto leadingStorage = Super::GetLeadingTypeStorage();
			if(leadingStorage == nullptr)
				return;

			((GetStorage<TypeIndex>() == leadingStorage ? CallFunctionForEachEntity<TypeIndex>(function, sequence) : void()), ...);
		}
	};

	/** @} */

	/** @addtogroup ECS-Internal
	 *  @{
	 */

	/** Provides helper functionality for a view containing single include types, and no excluded types. */
	template<typename StorageType>
	class TSingleStorageViewCommon
	{
	public:
		using Iterator = std::conditional_t<StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace, TViewIterator<1u, 0u, true>, typename StorageType::Iterator>;
		using ReverseIterator = std::conditional_t<StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace, void, typename StorageType::ReverseIterator>;

		/** Returns the included type storage pointer. */
		const StorageType* GetLeadingTypeStorage() const
		{
			return mStorage;
		}

		/** Returns a number of elements in the view based on the number of elements in the leading storage. */
		u64 GetSizeEstimate() const
		{
			if(mStorage == nullptr)
				return 0;

			return StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapOnly ? mStorage->GetFirstFreeElementPackedIndex() : mStorage->Size();
		}

		/** Iterator to the first entity matching the view filter. */
		Iterator Begin() const
		{
			if(mStorage == nullptr)
				return Iterator();

			if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace)
				return Iterator(mStorage->Begin(), { mStorage }, { }, 0);
			else
				return mStorage->Begin();
		}

		/** Iterator past the last entity matching the view filter. */
		Iterator End() const
		{
			if(mStorage == nullptr)
				return Iterator();

			if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapAndErase)
				return mStorage->End();
			else if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace)
				return Iterator(mStorage->End(), { mStorage }, { }, 0);
			else
				return mStorage->Begin() + mStorage->GetFirstFreeElementPackedIndex();
		}

		/** Iterator to the last entity matching the view filter. */
		template<SparseSetDeletePolicy DeletePolicy = StorageType::kDeletePolicy>
		std::enable_if_t<DeletePolicy != SparseSetDeletePolicy::InPlace, ReverseIterator> Rbegin() const
		{
			return mStorage != nullptr ? mStorage->Rbegin() : ReverseIterator();
		}

		/** Iterator before the first entity matching the view filter. */
		template<SparseSetDeletePolicy DeletePolicy = StorageType::kDeletePolicy>
		std::enable_if_t<DeletePolicy != SparseSetDeletePolicy::InPlace, ReverseIterator> Rend() const
		{
			if(mStorage == nullptr)
				return Iterator();

			if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapAndErase)
				return mStorage->Rend();
			else
				return mStorage != nullptr ? mStorage->Rbegin() + mStorage->GetFirstFreeElementPackedIndex() : ReverseIterator();
		}

		/** Reference to the first entity matching the view filter, or null if none matches. */
		Entity Front() const
		{
			auto it = Begin();
			return  it != End() ? *it : kNullEntity;
		}

		/** Reference to the last entity matching the view filter, or null if none matches. */
		Entity Back() const
		{
			if(mStorage == nullptr)
				return kNullEntity;

			auto it = mStorage->Rbegin();
			const auto last = it + GetSizeEstimate();

			while (it != last && *it == kInvalidEntity)
				++it;

			return it != last ? *it : kNullEntity;
		}

		/** Returns an iterator to the entity in the view, or End() iterator if no entity is found. */
		Iterator Find(Entity entity)
		{
			if(mStorage == nullptr)
				return End();

			if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapAndErase)
				return mStorage->Find(entity);
			else if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapOnly)
			{
				auto it = mStorage->Find(entity);
				return it->Index() < mStorage->GetFirstFreeElementPackedIndex() ? *it : End();
			}
			else
				return Iterator(mStorage->Find(entity), { mStorage }, {}, 0);
		}

		/** Returns true if the provided entity matches the view filter. */
		bool Contains(Entity entity)
		{
			if(mStorage == nullptr)
				return false;

			if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapAndErase || StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace)
				return mStorage->Contains(entity);
			else
				return mStorage->Contains(entity) && mStorage->GetPackedIndex(entity) < mStorage->GetFirstFreeElementPackedIndex();
		}

		/** Returns true if included storage pointer is initialized. View is not usable unless this returns true. */
		bool IsValid()
		{
			return mStorage != nullptr;
		}

		// For std compatibility
		using iterator = Iterator;
		using reverse_iterator = ReverseIterator;

		iterator begin() { return Begin(); }
		iterator end() { return End(); }
		reverse_iterator rbegin() { return Rbegin(); }
		reverse_iterator rend() { return Rend(); }

	protected:
		TSingleStorageViewCommon() = default;
		TSingleStorageViewCommon(const StorageType* storage)
			:mStorage(storage)
		{ }

	protected:
		const StorageType* mStorage = nullptr;
	};

	/** @} */

	/** @addtogroup ECS
	 *  @{
	 */

	/** Specialization of TView that is used when only a single type is provided for the included type filter. */
	template<typename StorageType>
	class TView<TIncludedTypes<StorageType>, TExcludedTypes<>> : public TSingleStorageViewCommon<StorageType>
	{
		using Super = TSingleStorageViewCommon<StorageType>;

	public:
		using IteratorRange = std::conditional_t<StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace, TIteratorRange<TViewIteratorAdapter<typename Super::Iterator, StorageType>>, decltype(std::declval<StorageType>().Each())>;

		TView() = default;
		TView(StorageType& storage)
			:Super::Super(&storage)
		{ }

		TView(std::tuple<StorageType&> includedTypes, std::tuple<> excludedTypes = {})
			:TView(std::get<0>(includedTypes))
		{ }

		/** Returns the storage associated with the view. */
		template<typename ElementType = typename StorageType::ElementType>
		auto* GetStorage() const
		{
			return GetStorage<0>();
		}

		/** Returns the storage associated with the view. */
		template<u32 Index>
		auto* GetStorage() const
		{
			static_assert(Index == 0, "Index out of range.");

			return const_cast<StorageType*>(Super::GetLeadingTypeStorage());
		}

		/** Assigns the storage associated with the view. */
		void SetStorage(StorageType& storage)
		{
			SetStorage<0>(storage);
		}

		/** Helper that assigns the storage at the specified index. */
		template<u32 Index>
		void SetStorage(StorageType& storage)
		{
			static_assert(Index == 0, "Index out of range.");

			this->mStorage = &storage;
		}

		decltype(auto) operator[](Entity entity) const
		{
			return GetStorage()->Get(entity);
		}

		/** Returns a component for the specified entity. */
		template<typename ElementType>
		decltype(auto) Get(Entity entity) const
		{
			static_assert(std::is_same_v<std::remove_const_t<ElementType>, typename StorageType::ElementType>, "Invalid element type.");
			return Get<0>(entity);
		}

		/**
		 * Returns a component for the specified entity. 
		 *
		 * @param	entity				Entity to retrieve the component for.
		 * @return						Tuple containing the requested component, if no indices are provided, or a single component of index is provided. Only index of zero is valid.
		 */
		template<u32... Index>
		decltype(auto) Get(Entity entity) const
		{
			if constexpr(sizeof...(Index) == 0)
				return GetAsTuple(GetStorage(), entity);
			else
				return GetStorage<Index...>()->Get(entity);
		}

		/** Allows easy iteration over the components in the view using a range for loop. */
		IteratorRange Each() const
		{
			if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace)
				return IteratorRange(Super::Begin(), Super::End());
			else
				return GetStorage() != nullptr ? GetStorage()->Each() : IteratorRange();
		}

		/**
		 * Calls @p function for each entry in the view. Valid signatures for @p function are:
		 *  (Entity, ComponentType&) - Passes both entity and the component to the function
		 *  (ComponentType&) - Passes only component to the function. Allows for fast iteration with as it only needs to iterate over the packed component storage.
		 *  () - Only allowed signature for iteration of storages containing empty types.
		 */
		template<typename Function>
		void DoForEach(const Function& function)
		{
			// Check for (Entity, Type&) signature, have to take the slow path in this case
			if constexpr(TIsInvocableWithTupleArguments<Function, decltype(std::tuple_cat(std::tuple<Entity>{}, std::declval<TView>().Get({})))>)
			{
				for(const auto entry : Super::GetLeadingTypeStorage()->Each())
					std::apply(function, entry);
			}
			// Check for (Type&) signature and try to use the fast path
			else
			{
				static_assert(TIsInvocableWithTupleArguments<Function, decltype(std::declval<TView>().Get({}))>, "Invalid function signature.");

				// Use fast path for swap-and-erase and swap-only
				if constexpr(StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapAndErase || StorageType::kDeletePolicy == SparseSetDeletePolicy::SwapOnly)
				{
					// For empty types no need to access the contents
					if constexpr(std::is_void_v<typename StorageType::ElementType>)
					{
						for(u64 index = 0; index < Super::GetSizeEstimate(); ++index)
							function();
					}
					// For non-empty types iterate over the packaged storage directly
					else
					{
						auto first = GetStorage()->Begin();
						auto last = GetStorage()->Begin() + Super::GetSizeEstimate();
						for(; first != last; ++first)
							function(*first);
					}
				}
				// For in-place delete we need the slow path as we need to check tombstones as we iterate
				else
				{
					for(const auto entry : Super::GetLeadingTypeStorage()->Each())
						std::apply(function, std::get<1>(entry));
				}
			}
		}
	};

	/** @} */
} // namespace b3d::ecs
