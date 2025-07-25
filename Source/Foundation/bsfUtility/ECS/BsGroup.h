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
#include "String/BsHashedString.h"

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
		constexpr TGroupIteratorAdapter(IteratorType iterator, std::tuple<OwnedStorageTypes*..., IncludedStorageTypes*...> ownedAndIncludedTypeStorage)
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

	struct GroupInternals
	{
		virtual ~GroupInternals() = default;

		// TODO - This needs to be implemented in derived types
		virtual bool OwnsType(const TypeId typeId) const { return false; }
	};

	template<u32 OwnedTypeCount, u32 IncludedTypeCount, u32 ExcludedTypeCount>
	struct TGroupInternals : GroupInternals
	{
		template<typename... OwnedAndIncludedTypes, typename... ExcludedTypes>
		TGroupInternals(const std::tuple<OwnedAndIncludedTypes...>& ownedAndIncludedTypes, const std::tuple<ExcludedTypes...>& excludedTypes)
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
		template<typename... IncludedTypes, typename... ExcludedTypes>
		TGroupInternals(const std::tuple<IncludedTypes...>& includedTypes, const std::tuple<ExcludedTypes...>& excludedTypes)
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

	// TODO - Move to utility
	constexpr std::string_view B3DGetTypeNameFromPrettyFunction()
	{
		const std::string_view prettyFunctionString(B3D_PRETTY_FUNCTION);
		auto typeNameStart = prettyFunctionString.find_first_not_of(' ', prettyFunctionString.find_first_of(B3D_PRETTY_FUNCTION_PREFIX) + 1);
		return prettyFunctionString.substr(typeNameStart, prettyFunctionString.find_last_of(B3D_PRETTY_FUNCTION_SUFFIX) - typeNameStart);
	}

	template<typename HashType = u64>
	constexpr HashType B3DGetCurrentTypeHash()
	{
		constexpr std::string_view typeName = B3DGetTypeNameFromPrettyFunction();
		return THashedString<char, HashType>::CalculateHash(typeName);
	}

	template<typename, typename, typename>
	class TGroup;

	template<typename... IncludedStorageTypes, typename... ExcludedStorageTypes>
	class TGroup<TOwnedTypes<>, TIncludedTypes<IncludedStorageTypes...>, TExcludedTypes<ExcludedStorageTypes...>>
	{
		template<u32 Index>
		using TStorageTypeAt = TTypeListElementAt<Index, TTypeList<IncludedStorageTypes..., ExcludedStorageTypes...>>;

		template<typename ElementType>
		static constexpr u32 TIndexOfElementType = TTypeListIndexOf<std::remove_const_t<ElementType>, TTypeList<typename IncludedStorageTypes::ElementType..., typename ExcludedStorageTypes::ElementType...>>;
		
	public:
		using Iterator = SparseSet::Iterator;
		using ReverseIterator = SparseSet::Iterator;
		using IteratorRange = TIteratorRange<TGroupIteratorAdapter<Iterator, TOwnedTypes<>, TIncludedTypes<IncludedStorageTypes...>>>;
		using GroupInternalsType = TGroupInternals<0, sizeof...(IncludedStorageTypes), sizeof...(ExcludedStorageTypes)>; 

		TGroup() = default;
		TGroup(GroupInternalsType& internals)
			:mInternals(internals)
		{ }

		const SparseSet& GetLeadingStorage()
		{
			return mInternals->GetGroupStorage();
		}

		template<typename ElementType>
		auto* GetStorage() const
		{
			return GetStorage<TIndexOfElementType<ElementType>>();
		}

		template<u32 Index>
		auto* GetStorage() const
		{
			static_assert(Index < (sizeof...(IncludedStorageTypes) + sizeof...(ExcludedStorageTypes)), "Index out of range.");

			using StorageType = TStorageTypeAt<Index>;
			return static_cast<StorageType*>(mInternals != nullptr ? mInternals->template GetStorage<Index>() : nullptr);
		}

		u64 GetSize() const
		{
			return mInternals != nullptr ? GetLeadingStorage().Size() : 0;
		}

		u64 GetCapacity() const
		{
			return mInternals != nullptr ? GetLeadingStorage().Capacity() : 0;
		}

		bool IsEmpty() const
		{
			return mInternals != nullptr ? mInternals->IsEmpty() : true;
		}

		void Shrink()
		{
			if(mInternals != nullptr) GetLeadingStorage().Shrink();
		}

		Iterator Begin() const { return mInternals != nullptr ? mInternals->Begin() : Iterator{}; }
		Iterator End() const { return mInternals != nullptr ? mInternals->End() : Iterator{}; }
		ReverseIterator Rbegin() const { return mInternals != nullptr ? mInternals->Rbegin() : ReverseIterator{}; }
		ReverseIterator Rend() const { return mInternals != nullptr ? mInternals->Rend() : ReverseIterator{}; }

		Entity Front() const
		{
			auto it = Begin();
			return it != End() ? *it : kNullEntity;
		}

		Entity Back() const
		{
			auto it = Rbegin();
			return it != Rend() ? *it : kNullEntity;
		}

		Iterator Find(Entity entity)
		{
			return mInternals != nullptr ? GetLeadingStorage().Find(entity) : Iterator{};
		}

		bool Contains(Entity entity)
		{
			return mInternals != nullptr ? GetLeadingStorage().Contains(entity) : false;
		}

		template<typename ElementType, typename... OtherElementType>
		decltype(auto) Get(Entity entity) const
		{
			return Get<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementType>...>(entity);
		}

		template<u32... Indices>
		decltype(auto) Get(Entity entity) const
		{
			const auto includedTypeStorage = GetIncludedStoragesAsTuple(std::index_sequence_for<IncludedStorageTypes...>{});

			if constexpr(sizeof...(Indices) == 0)
				return std::apply([entity](auto*... storage) { std::tuple_cat((GetAsTuple(storage->Get(entity)), ...)); }, includedTypeStorage);
			else if constexpr(sizeof...(Indices) == 1)
				return (std::get<Indices>(includedTypeStorage)->Get(entity), ...);
			else
				return std::tuple_cat(GetAsTuple(std::get<Indices>(includedTypeStorage), entity)...);
		}

		IteratorRange Each() const
		{
			const auto includedTypeStorage = GetIncludedStoragesAsTuple(std::index_sequence_for<IncludedStorageTypes...>{});
			return IteratorRange({ Begin(), includedTypeStorage}, { End(), includedTypeStorage });
		}

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

		template <u32... Indices, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			if(mInternals == nullptr)
				return;

			if constexpr(sizeof...(Indices) == 0)
			{
				static_assert(TIsInvocableWithTupleArguments<ComparisonFunction, const Entity, const Entity>, "Invalid comparison function");
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

		template <typename ElementType, typename... OtherElementTypes, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			Sort<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementTypes...>>(std::move(predicate));
		}

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

		static u64 TypeId()
		{
			return B3DGetCurrentTypeHash<u64>();
		}

		// For std compatibility
		using iterator = Iterator;
		using reverse_iterator = ReverseIterator;

		iterator begin() const { return Begin(); }
		iterator end() const { return End(); }
		reverse_iterator rbegin() const { return Rbegin(); }
		reverse_iterator rend() const { return Rend(); }

	private:
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

	template<typename... OwnedStorageTypes, typename... IncludedStorageTypes, typename... ExcludedStorageTypes>
	class TGroup<TOwnedTypes<OwnedStorageTypes...>, TIncludedTypes<IncludedStorageTypes...>, TExcludedTypes<ExcludedStorageTypes...>>
	{
		static_assert(((OwnedStorageTypes::kDeletePolicy != SparseSetDeletePolicy::InPlace) && ...), "In-place delete not supported for owned storage.");

		template<u32 Index>
		using TStorageTypeAt = TTypeListElementAt<Index, TTypeList<OwnedStorageTypes..., IncludedStorageTypes..., ExcludedStorageTypes...>>;

		template<typename ElementType>
		static constexpr u32 TIndexOfElementType = TTypeListIndexOf<std::remove_const_t<ElementType>, TTypeList<typename OwnedStorageTypes::ElementType..., typename IncludedStorageTypes::ElementType..., typename ExcludedStorageTypes::ElementType...>>;
		
	public:
		using Iterator = SparseSet::Iterator;
		using ReverseIterator = SparseSet::Iterator;
		using IteratorRange = TIteratorRange<TGroupIteratorAdapter<Iterator, TOwnedTypes<OwnedStorageTypes>..., TIncludedTypes<IncludedStorageTypes...>>>;
		using GroupInternalsType = TGroupInternals<sizeof...(OwnedStorageTypes), sizeof...(IncludedStorageTypes), sizeof...(ExcludedStorageTypes)>; 

		TGroup() = default;
		TGroup(GroupInternalsType& internals)
			:mInternals(internals)
		{ }

		const SparseSet& GetLeadingStorage()
		{
			return *GetStorage<0>();
		}

		template<typename ElementType>
		auto* GetStorage() const
		{
			return GetStorage<TIndexOfElementType<ElementType>>();
		}

		template<u32 Index>
		auto* GetStorage() const
		{
			static_assert(Index < (sizeof...(OwnedStorageTypes) + sizeof...(IncludedStorageTypes) + sizeof...(ExcludedStorageTypes)), "Index out of range.");

			using StorageType = TStorageTypeAt<Index>;
			return static_cast<StorageType*>(mInternals != nullptr ? mInternals->template GetStorage<Index>() : nullptr);
		}

		u64 GetSize() const
		{
			return mInternals != nullptr ? mInternals->Size() : 0;
		}

		bool IsEmpty() const
		{
			return mInternals != nullptr ? mInternals->Size() == 0 : true;
		}

		Iterator Begin() const { return mInternals != nullptr ? mInternals->Begin() : Iterator{}; }
		Iterator End() const { return mInternals != nullptr ? mInternals->Begin() + mInternals->Size() : Iterator{}; }
		ReverseIterator Rbegin() const { return mInternals != nullptr ? mInternals->Rbegin() + (GetLeadingStorage().Size() - mInternals->Size()) : ReverseIterator{}; }
		ReverseIterator Rend() const { return mInternals != nullptr ? mInternals->Rend() : ReverseIterator{}; }

		Entity Front() const
		{
			auto it = Begin();
			return it != End() ? *it : kNullEntity;
		}

		Entity Back() const
		{
			auto it = Rbegin();
			return it != Rend() ? *it : kNullEntity;
		}

		Iterator Find(Entity entity)
		{
			auto found = mInternals != nullptr ? GetLeadingStorage().Find(entity) : Iterator{};
			return found <= End() ? found : Iterator();
		}

		bool Contains(Entity entity)
		{
			return mInternals != nullptr ? (GetLeadingStorage().Contains(entity) && GetLeadingStorage().GetPackedIndex(entity) < mInternals->Size()) : false;
		}

		template<typename ElementType, typename... OtherElementType>
		decltype(auto) Get(Entity entity) const
		{
			return Get<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementType>...>(entity);
		}

		template<u32... Indices>
		decltype(auto) Get(Entity entity) const
		{
			const auto ownedAndIncludedTypeStorage = GetOwnedAndIncludedStoragesAsTuple(std::index_sequence_for<OwnedStorageTypes...>{}, std::index_sequence_for<IncludedStorageTypes...>{});

			if constexpr(sizeof...(Indices) == 0)
				return std::apply([entity](auto*... storage) { std::tuple_cat((GetAsTuple(storage->Get(entity)), ...)); }, ownedAndIncludedTypeStorage);
			else if constexpr(sizeof...(Indices) == 1)
				return (std::get<Indices>(ownedAndIncludedTypeStorage)->Get(entity), ...);
			else
				return std::tuple_cat(GetAsTuple(std::get<Indices>(ownedAndIncludedTypeStorage), entity)...);
		}

		IteratorRange Each() const
		{
			const auto ownedAndIncludedTypeStorage = GetOwnedAndIncludedStoragesAsTuple(std::index_sequence_for<OwnedStorageTypes...>{}, std::index_sequence_for<IncludedStorageTypes...>{});
			return IteratorRange({ Begin(), ownedAndIncludedTypeStorage}, { End(), ownedAndIncludedTypeStorage });
		}

		template<typename Function>
		void DoForEach(const Function& function)
		{
			for(auto entity : Each())
			{
				// Check for (Entity, Type&, ...) signature
				if constexpr(TIsInvocableWithTupleArguments<Function, decltype(std::tuple_cat(std::tuple<Entity>{}, std::declval<TGroup>().Get({})))>)
					std::apply(function, std::tuple_cat(std::forward_as_tuple(entity), Get(entity)));
				else // Check for (Type&, ...) signature (fast path)
					std::apply([&function](auto, auto&&... otherElements) { function(std::forward<decltype(otherElements)>(otherElements)...); } );
			}
		}

		template <u32... Indices, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			if(mInternals == nullptr)
				return;

			const auto ownedAndIncludedTypeStorage = GetOwnedAndIncludedStoragesAsTuple(std::index_sequence_for<OwnedStorageTypes...>{}, std::index_sequence_for<IncludedStorageTypes...>{});
			if constexpr(sizeof...(Indices) == 0)
			{
				static_assert(TIsInvocableWithTupleArguments<ComparisonFunction, const Entity, const Entity>, "Invalid comparison function");
				GetStorage<0>->SortN(mInternals->Size(), std::move(predicate));
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

				GetStorage<0>->SortN(mInternals->Size(), std::move(fnCompareElements));
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

		template <typename ElementType, typename... OtherElementTypes, typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = {})
		{
			Sort<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementTypes...>>(std::move(predicate));
		}

		Entity operator[](u64 index) const
		{
			return Begin()[index];
		}

		static u64 TypeId()
		{
			return B3DGetCurrentTypeHash<u64>();
		}

		// For std compatibility
		using iterator = Iterator;
		using reverse_iterator = ReverseIterator;

		iterator begin() const { return Begin(); }
		iterator end() const { return End(); }
		iterator rbegin() const { return Rbegin(); }
		iterator rend() const { return Rend(); }

	private:
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
