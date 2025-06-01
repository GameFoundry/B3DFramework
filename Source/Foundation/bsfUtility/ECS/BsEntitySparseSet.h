//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Debug/BsDebug.h"

#include <iterator>

namespace bs::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	// TODO - Move to utility library
	template<typename Type>
	constexpr std::enable_if_t<std::is_unsigned_v<Type>, u32> CountBits(Type value)
	{
		return value ? ((u32)(value & 1) + CountBits(static_cast<Type>(value >> 1))) : 0;
	}

	// TODO - Move to utility library
	using typeidfn_t = void(*)();
	using typeid_t = u64;

	//template <typename T>
	//static typeid_t type_id() noexcept
	//{
	//  return (typeid_t)(typeidfn_t(type_id<T>));
	//}

	template <typename T>
	typeid_t type_id() noexcept
	{
	  static char const type_id = 0;

	  return (u64)&type_id;
	}

	template<typename>
	struct TEntityTypeTraits;

	template<>
	struct TEntityTypeTraits<u32>
	{
		using StorageType = u32;
		using IdentifierType = u32;
		using VersionType = u16;

		static constexpr u32 kIdentifierBitCount = 20;
		static constexpr u32 kVersionBitCount  = 12;
		static constexpr StorageType kIdentifierMask = (1 << kIdentifierBitCount) - 1;
		static constexpr StorageType kVersionMask = (1 << kVersionBitCount) - 1;
	};

	template<>
	struct TEntityTypeTraits<u64>
	{
		using StorageType = u64;
		using IdentifierType = u64;
		using VersionType = u32;

		static constexpr u32 kIdentifierBitCount = 32;
		static constexpr u32 kVersionBitCount  = 32;
	};

	template<typename Type>
	struct TEntity
	{
		using Traits = TEntityTypeTraits<Type>;
		using StorageType = typename Traits::StorageType;
		using IdentifierType = typename Traits::IdentifierType;
		using VersionType = typename Traits::VersionType;

		static constexpr StorageType kIdentifierMask = (1 << Traits::kIdentifierBitCount) - 1;
		static constexpr StorageType kVersionMask = (1 << Traits::kVersionBitCount) - 1;

		constexpr TEntity() = default;
		constexpr TEntity(IdentifierType identifier, VersionType version)
		{
			if constexpr(Traits::kVersionMask == 0u)
				IdentifierAndVersion = (Type)(identifier & Traits::kIdentifierMask);
			else
				IdentifierAndVersion = ((Type)(identifier & Traits::kIdentifierMask)) | (((Type)(version & Traits::kVersionMask)) << Traits::kIdentifierBitCount);
		}

		bool operator==(TEntity other) const
		{
			return IdentifierAndVersion == other.IdentifierAndVersion;
		}

		bool operator!=(TEntity other) const
		{
			return IdentifierAndVersion != other.IdentifierAndVersion;
		}

		bool operator<(TEntity other) const
		{
			return Identifier < other.Identifier;
		}

		constexpr IdentifierType GetIdentifier() const
		{
			return (IdentifierType)(IdentifierAndVersion) & Traits::kIdentifierMask;
		}

		constexpr VersionType GetVersion() const
		{
			if constexpr(Traits::kVersionMask == 0u)
				return VersionType{};

			return (Traits::VersionType)(IdentifierAndVersion >> Traits::kIdentifierBitCount) & Traits::kVersionMask;
		}

		constexpr TEntity GetAsNextVersion() const
		{
			return TEntity(GetIdentifier(), GetVersion() + 1);
		}
		
		union
		{
			struct
			{
				StorageType Identifier : Traits::kIdentifierBitCount;
				StorageType Version : Traits::kVersionBitCount;
			};

			StorageType IdentifierAndVersion;
		};
	};

#define B3D_ECS_64BIT_ENTITIES 0
#define B3D_ECS_SPARSE_SET_PAGE_SIZE 4096

#if B3D_ECS_64BIT_ENTITIES
	using Entity = TEntity<u64>;
#else
	using Entity = TEntity<u32>;
#endif

	// TODO - Doc
	struct NullEntity
	{
		template<typename Type>
		constexpr operator TEntity<Type>() const
		{
			return TEntity<Type>(TEntity<Type>::Traits::kIdentifierMask, TEntity<Type>::Traits::kVersionMask);
		}

		constexpr bool operator==(NullEntity) const
		{
			return true;
		}

		constexpr bool operator!=(NullEntity) const
		{
			return false;
		}

		template<typename Type>
		constexpr bool operator==(TEntity<Type> value) const
		{
			return value.GetIdentifier() == ((TEntity<Type>)*this).GetIdentifier();
		}

		template<typename Type>
		constexpr bool operator!=(TEntity<Type> value) const
		{
			return !(value == *this);
		}
	};

	template<typename Type>
	constexpr bool operator==(TEntity<Type> lhs, NullEntity rhs)
	{
		return rhs.operator==(lhs);
	}

	template<typename Type>
	constexpr bool operator!=(TEntity<Type> lhs, NullEntity rhs)
	{
		return !(rhs == lhs);
	}

	// TODO - Doc
	struct InvalidEntity
	{
		template<typename Type>
		constexpr operator TEntity<Type>() const
		{
			return TEntity<Type>(TEntity<Type>::Traits::kIdentifierMask, TEntity<Type>::Traits::kVersionMask);
		}

		constexpr bool operator==(InvalidEntity) const
		{
			return true;
		}

		constexpr bool operator!=(InvalidEntity) const
		{
			return false;
		}

		template<typename Type>
		constexpr bool operator==(TEntity<Type> value) const
		{
			if constexpr(TEntityTypeTraits<Type>::kVersionMask == 0u)
				return false;

			return value.GetVersion() == ((TEntity<Type>)*this).GetVersion();
		}

		template<typename Type>
		constexpr bool operator!=(Type value) const
		{
			return !(value == *this);
		}
	};

	template<typename Type>
	constexpr bool operator==(Type lhs, InvalidEntity rhs)
	{
		return rhs.operator==(lhs);
	}

	template<typename Type>
	constexpr bool operator!=(Type lhs, InvalidEntity rhs)
	{
		return !(rhs == lhs);
	}

	inline constexpr NullEntity kNullEntity;
	inline constexpr InvalidEntity kInvalidEntity;

	template<typename ContainerType>
	struct TContiguousContainerIterator final
	{
		using value_type = typename ContainerType::value_type;
		using pointer = typename ContainerType::const_pointer;
		using reference = typename ContainerType::const_reference;
		using difference_type = typename ContainerType::difference_type;
		using const_pointer = typename ContainerType::const_pointer;
		using const_reference = typename ContainerType::const_reference;
		using iterator_category = std::random_access_iterator_tag;

		TContiguousContainerIterator() = default;
		TContiguousContainerIterator(const ContainerType& container, u64 index)
			: mContainer(&container), mIndex(index) { }

		TContiguousContainerIterator& operator++()
		{
			++mIndex;
			return *this;
		}

		TContiguousContainerIterator& operator--()
		{
			B3D_ENSURE(mIndex > 0);

			--mIndex;
			return *this;
		}

		TContiguousContainerIterator& operator+=(u64 value)
		{
			mIndex += value;
			return *this;
		}

		TContiguousContainerIterator& operator-=(u64 value)
		{
			B3D_ENSURE(mIndex >= value);

			mIndex -= value;
			return *this;
		}

		TContiguousContainerIterator operator+(u64 value) const
		{
			TContiguousContainerIterator copy = *this;
			return (copy += value);
		}

		TContiguousContainerIterator operator-(u64 value) const
		{
			TContiguousContainerIterator copy = *this;
			return (copy -= value);
		}

		const_reference operator[](u64 value) const { return (*mContainer)[mIndex + value]; }
		const_pointer operator->() const { return std::addressof(operator[](0)); }
		const_reference operator*() const { return operator[](0); }

		u64 Index() const { return mIndex; }

	private:
		const ContainerType* mContainer = nullptr;
		u64 mIndex = 0;
	};

	template<typename ContainerType>
	i64 operator-(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return rhs.Index() - lhs.Index();
	}

	template<typename ContainerType>
	bool operator==(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return lhs.Index() == rhs.Index();
	}

	template<typename ContainerType>
	bool operator!=(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename ContainerType>
	bool operator<(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return lhs.index() > rhs.index();
	}

	template<typename ContainerType>
	bool operator>(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return rhs < lhs;
	}

	template<typename ContainerType>
	bool operator<=(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return !(lhs > rhs);
	}

	template<typename ContainerType>
	bool operator>=(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return !(lhs < rhs);
	}

	// TODO - Doc
	enum class SparseSetDeletePolicy
	{
		InPlace,
		SwapAndErase,
		SwapOnly
	};

	class SparseSet
	{
		using SparseContainerType = TArray<TArrayView<Entity>>;
		using PackedContainerType = TArray<Entity>;
		static constexpr u32 SparsePageSize = B3D_ECS_SPARSE_SET_PAGE_SIZE;

		static_assert(SparsePageSize > 0 && (SparsePageSize & SparsePageSize - 1) == 0, "SparsePageSize must be a power of two value.");

	public:
		using Iterator = TContiguousContainerIterator<PackedContainerType>;
		using ConstIterator = Iterator;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

		static constexpr u64 kMaximumEntryCount = Entity::Traits::kIdentifierMask;

		SparseSet() = default;
		virtual ~SparseSet()
		{
			FreeSparsePages();
		}

		Entity operator[](u64 index)
		{
			return mPackedEntities[index];
		}

		Iterator Add(Entity entity) // TODO - Make protected?
		{
			return AddInternal(entity, false);
		}

		Iterator Add(Iterator first, Iterator last) // TODO - Make protected?
		{
			Iterator iterator = End();
			for(; first != last; ++first)
				iterator = AddInternal(*first, true);

			return iterator;
		}

		void Erase(Entity entity)
		{
			EraseInternal(entity);
		}

		void Erase(Iterator first, Iterator last)
		{
			for(; first != last; ++first)
				EraseInternal(*first);
		}

		bool EraseIfValid(Entity entity)
		{
			if(Contains(entity))
			{
				Erase(entity);
				return true;
			}

			return false;
		}

		u64 EraseIfValid(Iterator first, Iterator last)
		{
			u64 removedEntryCount = 0;

			while(first != last)
			{
				while(first != last && !Contains(*first))
					++first;

				const auto iterator = first;

				while(first != last && Contains(*first))
					++first;

				removedEntryCount += (u64)std::distance(iterator, first);
				Erase(iterator, first);
			}

			return removedEntryCount;
		}

		virtual void Clear()
		{
			FreeSparsePages();
			mPackedEntities.Clear();
		}

		virtual void ClearInvalid()
		{
		}

		virtual void Shrink()
		{
			const u64 maximumPageCount = (u64)mSparseIndices.Size();
			u64 createdPageCount = 0;

			SparseContainerType newSparseIndices;
			newSparseIndices.Reserve(maximumPageCount);

			for(const auto& entity : mPackedEntities)
			{
				if(entity == kInvalidEntity)
					continue;

				const u64 entityIdentifier = entity.GetIdentifier();
				const u64 sparsePage = GetSparsePage(entityIdentifier);

				if(mSparseIndices[sparsePage].IsEmpty())
					continue;

				if(sparsePage >= (u64)newSparseIndices.Size())
					newSparseIndices.Resize(sparsePage + 1);

				newSparseIndices[sparsePage] = std::exchange(mSparseIndices[sparsePage], {});
				createdPageCount++;

				// Early exit
				if(createdPageCount == maximumPageCount)
					break;
			}

			FreeSparsePages();
			mSparseIndices = std::move(newSparseIndices);

			mSparseIndices.Shrink();
			mPackedEntities.Shrink();
		}

		virtual void Reserve(u64 capacity)
		{
			if(capacity == 0)
				return;

			const u64 requiredPageCapacity = GetSparsePage(capacity - 1) + 1;
			if(requiredPageCapacity > (u64)mSparseIndices.Size())
				mSparseIndices.Resize(requiredPageCapacity);

			mPackedEntities.Reserve(capacity);
		}

		virtual u64 Capacity() const
		{
			return mPackedEntities.Capacity();
		}

		Entity::VersionType GetVersion(Entity entity) const
		{
			Entity* const sparseEntry = GetSparseEntryPointer(entity);
			if(sparseEntry == nullptr)
				return ((Entity)kInvalidEntity).GetVersion();
			
			return sparseEntry->GetVersion();
		}

		void UpdateVersion(Entity entity)
		{
			Entity& sparseEntry = GetSparseEntryReference(entity);
			sparseEntry = Entity(sparseEntry.GetIdentifier(), entity.GetVersion());

			const u64 packedEntryIndex = sparseEntry.GetIdentifier();
			mPackedEntities[packedEntryIndex] = entity;
		}

		Iterator Find(Entity entity) const
		{
			return Contains(entity) ? GetIterator(entity) : End();
		}

		bool Contains(Entity entity) const
		{
			Entity* const sparseEntry = GetSparseEntryPointer(entity);
			if(sparseEntry == nullptr)
				return false;

			if(sparseEntry->GetVersion() != entity.GetVersion())
				return false;

			return true;
		}

		u64 GetPackedIndex(Entity entity) const
		{
			return GetSparseEntryReference(entity).GetIdentifier();
		}

		// Note: For in-place deletion, this will also return deleted entries
		u64 Size() const { return mPackedEntities.Size(); }
		bool IsEmpty() const { return mPackedEntities.Empty(); }
		virtual SparseSetDeletePolicy GetDeletePolicy() const { return SparseSetDeletePolicy::SwapAndErase; }
		virtual u64 GetFreeListHead() const { return kMaximumEntryCount; }

		Iterator Begin() const { return Iterator(mPackedEntities, 0); }
		ConstIterator Cbegin() const { return Begin(); }

		Iterator End() const { return Iterator(mPackedEntities, Size()); }
		ConstIterator Cend() const { return End(); }

		ReverseIterator Rbegin() const { return std::make_reverse_iterator(End()); }
		ConstReverseIterator Crbegin() const { return Rbegin(); }

		ReverseIterator Rend() const { return std::make_reverse_iterator(Begin()); }
		ConstReverseIterator Crend() const { return Rend(); }

		// For std compatibility
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using reverse_iterator = ReverseIterator;
		using const_reverse_iterator = ConstReverseIterator;

		iterator begin() const { return Begin(); }
		const_iterator cbegin() const { return Cbegin(); }

		iterator end() const { return End(); }
		const_iterator cend() const { return Cend(); }

		reverse_iterator rbegin() const { return Rend(); }
		const_reverse_iterator crbegin() const { return Crbegin(); }

		reverse_iterator rend() const { return Rend(); }
		const_reverse_iterator crend() const { return Crend(); }

	protected:
		virtual Iterator AddInternal(Entity entity, bool forceAddAtEnd) { return End(); }
		virtual void EraseInternal(Entity entity) { }

		void SwapEntities(u64 lhsPackedIndex, u64 rhsPackedIndex)
		{
			Entity& lhsPackedEntry = mPackedEntities[lhsPackedIndex];
			Entity& rhsPackedEntry = mPackedEntities[rhsPackedIndex];

			Entity& lhsSparseEntry = GetSparseEntryReference(lhsPackedEntry);
			Entity& rhsSparseEntry = GetSparseEntryReference(rhsPackedEntry);

			lhsSparseEntry = Entity((Entity::Traits::IdentifierType)rhsPackedIndex, lhsPackedEntry.GetVersion());
			rhsSparseEntry = Entity((Entity::Traits::IdentifierType)lhsPackedIndex, rhsPackedEntry.GetVersion());

			std::swap(lhsPackedEntry, rhsPackedEntry);
		}

		Entity& GetOrCreateSparseEntryReference(Entity entity)
		{
			const u64 entityIdentifier = entity.GetIdentifier();
			const u64 sparsePage = GetSparsePage(entityIdentifier);

			if(sparsePage >= mSparseIndices.Size())
				mSparseIndices.Resize(sparsePage + 1);

			if(mSparseIndices[sparsePage].IsEmpty())
			{
				mSparseIndices[sparsePage] = TArrayView<Entity>(B3DAllocateMultiple<Entity>(SparsePageSize), SparsePageSize);
				std::uninitialized_fill(mSparseIndices[sparsePage].Data(), mSparseIndices[sparsePage].Data() + SparsePageSize, kNullEntity);
			}

			return mSparseIndices[sparsePage][GetSparseIndexWithinPage(entityIdentifier)];
		}

		Entity& GetSparseEntryReference(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			return const_cast<Entity&>(mSparseIndices[GetSparsePage(entityIdentifier)][GetSparseIndexWithinPage(entityIdentifier)]);
		}

		Entity* GetSparseEntryPointer(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			const u64 sparsePage = GetSparsePage(entityIdentifier);

			if(sparsePage < mSparseIndices.Size() && !mSparseIndices[sparsePage].IsEmpty())
				return const_cast<Entity*>(&mSparseIndices[sparsePage][GetSparseIndexWithinPage(entityIdentifier)]);

			return nullptr;
		}

		Iterator GetIterator(Entity entity) const
		{
			return Iterator(mPackedEntities, (i64)GetPackedIndex(entity));
		}

		void FreeSparsePages()
		{
			for(auto&& page : mSparseIndices)
			{
				if(!page.IsEmpty())
				{
					std::destroy(page.Data(), page.Data() + SparsePageSize);
					B3DFree(page.Data());
					page = {};
				}
			}
			mSparseIndices.Clear();
		}

		static constexpr Entity::IdentifierType GetPackedIndexAsEntryIdentifier(u64 packedIndex)
		{
			return (Entity::IdentifierType)packedIndex;
		}

		static constexpr u64 GetSparseIndexWithinPage(u64 entityIdentifier)
		{
			return entityIdentifier & (SparsePageSize - 1);
		}

		static constexpr u64 GetSparsePage(u64 entityIdentifier)
		{
			return entityIdentifier / SparsePageSize;
		}

		SparseContainerType mSparseIndices; // Entity identifiers are actually indexes into the packed array
		PackedContainerType mPackedEntities; // TODO - Might consider paging this. It won't be continous anymore, making it harder to iterate, but adding entries might prevent expensive resizes if there's a lot of entries
	};

	template<SparseSetDeletePolicy DeletePolicy>
	class TSparseSet : public SparseSet
	{
		using Super = SparseSet;
	public:
		static constexpr SparseSetDeletePolicy kDeletePolicy = DeletePolicy;

		TSparseSet() = default;
		~TSparseSet() override = default;

		void Clear() override
		{
			Super::Clear();
			mFreeListHead = DeletePolicy != SparseSetDeletePolicy::SwapOnly ? Super::kMaximumEntryCount : 0;
		}

		void ClearInvalid() override
		{
			ClearInvalidInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload>();
		}

		template<typename It>
		Iterator SortAs(It first, It last)
		{
			return SortAsInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload>(first, last);
		}

		template<typename ComparisonFunction = std::less<>>
		void SortN(u64 count)
		{
			return SortNInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload, ComparisonFunction>(count);
		}

		template<typename ComparisonFunction = std::less<>>
		void Sort()
		{
			return SortInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload, ComparisonFunction>();
		}

		SparseSetDeletePolicy GetDeletePolicy() const override { return DeletePolicy; }
		u64 GetFreeListHead() const override { return mFreeListHead; }

	protected:
		Iterator AddInternal(Entity entity, bool forceAddAtEnd) override
		{
			Entity& sparseSetEntry = GetOrCreateSparseEntryReference(entity);
			u64 packedEntryIndex = mPackedEntities.Size();

			if constexpr(DeletePolicy == SparseSetDeletePolicy::InPlace)
			{
				B3D_ENSURE(sparseSetEntry == kNullEntity);

				if(mFreeListHead != kMaximumEntryCount && !forceAddAtEnd)
				{
					packedEntryIndex = mFreeListHead;
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(mFreeListHead), entity.GetVersion());
					mFreeListHead = (u64)(std::exchange(mPackedEntities[packedEntryIndex], entity).GetIdentifier());
				}
				else
				{
					mPackedEntities.Add(entity);
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
				}
			}
			else if constexpr(DeletePolicy == SparseSetDeletePolicy::SwapAndErase)
			{
				mPackedEntities.Add(entity);
				B3D_ENSURE(sparseSetEntry == kNullEntity);
				sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
			}
			else
			{
				if(sparseSetEntry == kNullEntity)
				{
					mPackedEntities.Add(entity);
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
				}
				else
				{
					B3D_ENSURE(!(sparseSetEntry.GetIdentifier() < mFreeListHead));
					sparseSetEntry = sparseSetEntry.GetAsNextVersion();
				}

				packedEntryIndex = mFreeListHead++;
				SwapEntities(sparseSetEntry.GetIdentifier(), packedEntryIndex);
			}

			return Begin() + packedEntryIndex;
		}

		void EraseInternal(Entity entity) override
		{
			if constexpr(DeletePolicy == SparseSetDeletePolicy::InPlace)
			{
				// Sparse entry is set to a null value, while packed entry points to the next free packed entry, and its marked as invalid via its version
				const u64 packedEntryIndex = std::exchange(GetSparseEntryReference(entity), kNullEntity).GetIdentifier();
				mPackedEntities[packedEntryIndex] = Entity(GetPackedIndexAsEntryIdentifier(std::exchange(mFreeListHead, packedEntryIndex)), ((Entity)kInvalidEntity).GetVersion());
			}
			else if constexpr(DeletePolicy == SparseSetDeletePolicy::SwapAndErase)
			{
				// Set last sparse entry so it points to the packed index of the entry that was removed, swap packed entries
				Entity& sparseEntryToRemove = GetSparseEntryReference(entity);
				Entity& lastSparseEntry = GetSparseEntryReference(mPackedEntities.Back());

				const u64 packedEntryToRemoveIndex = sparseEntryToRemove.GetIdentifier();
				lastSparseEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryToRemoveIndex), mPackedEntities.Back().GetVersion());
				mPackedEntities[packedEntryToRemoveIndex] = mPackedEntities.Back();

				sparseEntryToRemove = kNullEntity;
				mPackedEntities.Pop();
			}
			else
			{
				const u64 packedEntryIndex = GetPackedIndex(entity);
				UpdateVersion(entity.GetAsNextVersion());

				mFreeListHead -= (packedEntryIndex < mFreeListHead);
				SwapEntities(packedEntryIndex, mFreeListHead);
			}
		}

		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64)>
		void ClearInvalidInternal()
		{
			if constexpr(DeletePolicy != SparseSetDeletePolicy::InPlace)
				return;

			u64 validPackedEntryIndex = mPackedEntities.Size();
			u64 freePackedEntryIndex = std::exchange(mFreeListHead, kMaximumEntryCount);

			// Find first valid entry
			for(; validPackedEntryIndex > 0 && mPackedEntities[validPackedEntryIndex - 1] == kInvalidEntity; --validPackedEntryIndex) { }

			while(freePackedEntryIndex != kMaximumEntryCount)
			{
				if(freePackedEntryIndex < validPackedEntryIndex)
				{
					--validPackedEntryIndex;

					// Move the free entry to the back
					Entity& fromPackedEntry = mPackedEntities[validPackedEntryIndex];
					Entity& toPackedEntry = mPackedEntities[freePackedEntryIndex];

					(((T*)this)->*MoveOrSwapPayload)(validPackedEntryIndex, freePackedEntryIndex);
					GetSparseEntryReference(fromPackedEntry) = Entity((Entity::Traits::IdentifierType)freePackedEntryIndex, toPackedEntry.GetVersion());

					// Find next free entry
					freePackedEntryIndex = mPackedEntities[freePackedEntryIndex].GetIdentifier();

					toPackedEntry = fromPackedEntry;

					// Find next valid entry
					for(; validPackedEntryIndex > 0 && mPackedEntities[validPackedEntryIndex - 1] == kInvalidEntity; --validPackedEntryIndex) { }
				}
				// Already at the end
				else
				{
					// Find next free entry
					freePackedEntryIndex = mPackedEntities[freePackedEntryIndex].GetIdentifier();
				}
			}

			mPackedEntities.Erase(mPackedEntities.begin() + validPackedEntryIndex, mPackedEntities.end());
		}

		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64), typename It>
		Iterator SortAsInternal(It first, It last)
		{
			if(!B3D_ENSURE(GetDeletePolicy() != SparseSetDeletePolicy::InPlace || mFreeListHead == Super::kMaximumEntryCount))
				return Begin();

			auto localIterator = Begin();
			const u64 validPackedEntryCount = (GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly) ? mFreeListHead : mPackedEntities.size();

			for(const auto localLast = Begin() + validPackedEntryCount; (localIterator != localLast) && (first != last); ++first)
			{
				if(auto entity = *first; Contains(entity))
				{
					if(auto localEntity = *localIterator; localEntity != entity)
					{
						const u64 packedIndexFrom = GetPackedIndex(localEntity);
						const u64 packedIndexTo = GetPackedIndex(entity);

						(((T*)this)->*MoveOrSwapPayload)(packedIndexFrom, packedIndexTo);
						SwapEntities(packedIndexFrom, packedIndexTo);
					}

					++localIterator;
				}
			}

			return localIterator;
		}

		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64), typename ComparisonFunction = std::less<>>
		void SortNInternal(u64 count, ComparisonFunction predicate = ComparisonFunction{})
		{
			if(!B3D_ENSURE(GetDeletePolicy() != SparseSetDeletePolicy::InPlace || mFreeListHead == Super::kMaximumEntryCount))
				return;

			if(!B3D_ENSURE(count <= mPackedEntities.Size()))
				return;

			std::sort(mPackedEntities.Begin(), mPackedEntities.End(), std::move(predicate));

			//for(u64 packedIndex = 0; packedIndex < count; ++packedIndex)
			//{
			//	u64 originalPackedIndex = GetPackedIndex(mPackedEntities[packedIndex]);
			//	if(packedIndex == originalPackedIndex)
			//		continue;

			//	const Entity entity = mPackedEntities[packedIndex];

			//	GetSparseEntryReference(entity) = Entity(GetPackedIndexAsEntryIdentifier(packedIndex), entity.GetVersion());
			//	(((T*)this)->*MoveOrSwapPayload)(originalPackedIndex, packedIndex);
			//}

			for(u64 rootPackedIndexToCheck = 0; rootPackedIndexToCheck < count; ++rootPackedIndexToCheck)
			{
				u64 packedIndexToCheck = rootPackedIndexToCheck;
				u64 originalPackedIndex = GetPackedIndex(mPackedEntities[packedIndexToCheck]);

				while(packedIndexToCheck != originalPackedIndex)
				{
					const u64 nextPackedIndex = GetPackedIndex(mPackedEntities[originalPackedIndex]);
					const Entity entity = mPackedEntities[packedIndexToCheck];

					(((T*)this)->*MoveOrSwapPayload)(originalPackedIndex, nextPackedIndex);

					// Make sure the sparse entry points to the new index
					GetSparseEntryReference(entity) = Entity(GetPackedIndexAsEntryIdentifier(packedIndexToCheck), mPackedEntities[packedIndexToCheck].GetVersion());
					packedIndexToCheck = std::exchange(originalPackedIndex, nextPackedIndex);
				}
			}
		}

		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64), typename ComparisonFunction = std::less<>>
		void SortInternal(ComparisonFunction predicate = ComparisonFunction{})
		{
			const u64 validPackedEntryCount = (GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly) ? mFreeListHead : mPackedEntities.size();
			SortNInternal<T, MoveOrSwapPayload>(validPackedEntryCount, std::move(predicate));
		}

	private:
		void MoveOrSwapPayload(u64, u64)
		{
			// Do nothing
		}

		/**
		 * For in-place delete policy points to first free entry, or kMaximumEntryCount if no free entries.
		 * For swap-only delete policy points to the first free entry, where all other elements are sequentially following the first free element. This value corresponds to valid entry count.
		 * For swap-and-erase delete policy this value is not used.
		 */
		u64 mFreeListHead = DeletePolicy != SparseSetDeletePolicy::SwapOnly ? Super::kMaximumEntryCount : 0;
	};

 	template<typename ContainerType, u32 PageSize>
	struct TPagedContainerIterator final
	{
		using value_type = typename ContainerType::value_type;
		using pointer = typename std::pointer_traits<typename ContainerType::value_type>::element_type*;
		using reference = typename std::pointer_traits<typename ContainerType::value_type>::element_type&;
		using difference_type = typename std::pointer_traits<typename ContainerType::value_type>::difference_type;
		using iterator_category = std::random_access_iterator_tag;

		TPagedContainerIterator() = default;
		TPagedContainerIterator(ContainerType& container, u64 index)
			: mContainer(&container), mIndex(index) { }

		TPagedContainerIterator& operator++()
		{
			++mIndex;
			return *this;
		}

		TPagedContainerIterator& operator--()
		{
			B3D_ENSURE(mIndex > 0);

			--mIndex;
			return *this;
		}

		TPagedContainerIterator& operator+=(u64 value)
		{
			mIndex += value;
			return *this;
		}

		TPagedContainerIterator& operator-=(u64 value)
		{
			B3D_ENSURE(mIndex >= value);

			mIndex -= value;
			return *this;
		}

		TPagedContainerIterator operator+(u64 value) const
		{
			TPagedContainerIterator copy = *this;
			return (copy += value);
		}

		TPagedContainerIterator operator-(u64 value) const
		{
			TPagedContainerIterator copy = *this;
			return (copy -= value);
		}

		reference operator[](u64 value) const
		{
			const u64 index = mIndex + value;
			return (*mContainer)[index / PageSize][index & (PageSize - 1)];
		}
		pointer operator->() const { return std::addressof(operator[](0)); }
		reference operator*() const { return operator[](0); }

		u64 Index() const { return mIndex; }

	private:
		ContainerType* mContainer = nullptr;
		u64 mIndex = 0;
	};

	template<typename ContainerType, u32 PageSize>
	i64 operator-(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return rhs.Index() - lhs.Index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator==(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return lhs.Index() == rhs.Index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator!=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename ContainerType, u32 PageSize>
	bool operator<(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return lhs.index() > rhs.index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator>(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return rhs < lhs;
	}

	template<typename ContainerType, u32 PageSize>
	bool operator<=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs > rhs);
	}

	template<typename ContainerType, u32 PageSize>
	bool operator>=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs < rhs);
	}

	template<typename ComponentType, bool InPlaceDelete = false, u64 PackedPageSize = 1024>
	class TComponentSparseSet : public TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase>
	{
		using ComponentContainerType = TArray<TArrayView<ComponentType>>;

	public:
		using ElementType = ComponentType;
		using Super = TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase>;
		using Iterator = TPagedContainerIterator<ComponentContainerType, PackedPageSize>;
		using ConstIterator = TPagedContainerIterator<const ComponentContainerType, PackedPageSize>;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

		~TComponentSparseSet() override
		{
			ShrinkComponentArray(0);
		}

		template<typename... Arguments>
		ComponentType& Add(Entity entity, Arguments&&... arguments)
		{
			auto iterator = AddInternal(entity, false, std::forward<Arguments>(arguments)...);
			return GetComponentReference(iterator.Index());
		}

		template<typename It>
		void Add(It first, It last, const ComponentType& value)
		{
			for(It it = first; it != last; ++it)
				AddInternal(*it, true, value);
		}

		ComponentType& Get(Entity entity)
		{
			const u64 packedEntryIndex = Super::GetPackedIndex(entity);
			return GetComponentReference(packedEntryIndex);
		}

		const ComponentType& Get(Entity entity) const
		{
			const u64 packedEntryIndex = Super::GetPackedIndex(entity);
			return GetComponentReference(packedEntryIndex);
		}

		void Clear() override
		{
			if constexpr(InPlaceDelete)
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
				{
					if(*it != kInvalidEntity)
					{
						ComponentType& component = GetComponentReference(GetPackedIndex(*it));
						component.~ComponentType();
					}
				}
			}
			else
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
				{
					ComponentType& component = GetComponentReference(GetPackedIndex(*it));
					component.~ComponentType();
				}
			}

			Super::Clear();
		}

		void ClearInvalid() override
		{
			Super::template ClearInvalidInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload>();
		}

		template<typename It>
		Iterator SortAs(It first, It last)
		{
			return Super::template SortAsInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload>(first, last);
		}

		template<typename ComparisonFunction = std::less<>>
		void SortN(u64 count)
		{
			return Super::template SortNInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload, ComparisonFunction>(count);
		}

		template<typename ComparisonFunction = std::less<>>
		void Sort()
		{
			return Super::template SortInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload, ComparisonFunction>();
		}

		void Reserve(u64 capacity) override
		{
			if(capacity > 0)
			{
				Super::Reserve(capacity);
				GetOrCreateComponentPageFor(capacity - 1u);
			}
		}

		u64 Capacity() const override
		{
			return mComponents.Size() * PackedPageSize;
		}

		void Shrink() override
		{
			Super::Shrink();
			ShrinkComponentArray(Super::Size());
		}

		ConstIterator Cbegin() const { return ConstIterator(mComponents, 0); }
		ConstIterator Begin() const { return Cbegin(); }
		Iterator Begin() { return Iterator(mComponents, 0); }

		ConstIterator Cend() const { return ConstIterator(mComponents, Super::Size()); }
		ConstIterator End() const { return Cend(); }
		Iterator End() { return Iterator(mComponents, Super::Size()); }

		ConstReverseIterator Crbegin() const { return std::make_reverse_iterator(Cend()); }
		ConstReverseIterator Rbegin() const { return Crbegin(); }
		ReverseIterator Rbegin() { return std::make_reverse_iterator(End()); }

		ConstReverseIterator Crend() const { return std::make_reverse_iterator(Cbegin()); }
		ConstReverseIterator Rend() const { return Crend(); }
		ReverseIterator Rend() { return std::make_reverse_iterator(Begin()); }

		// For std compatibility
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using reverse_iterator = ReverseIterator;
		using const_reverse_iterator = ConstReverseIterator;

		const_iterator cbegin() const { return Cbegin(); }
		const_iterator begin() const { return Begin(); }
		iterator begin() { return Begin(); }

		const_iterator cend() const { return Cend(); }
		const_iterator end() const { return End(); }
		iterator end() { return End(); }

		const_reverse_iterator crbegin() const { return Crbegin(); }
		const_reverse_iterator rbegin() const { return Rbegin(); }
		reverse_iterator rbegin() { return Rend(); }

		const_reverse_iterator crend() const { return Crend(); }
		const_reverse_iterator rend() const { return Rend(); }
		reverse_iterator rend() { return Rend(); }

	private:
		using UnderlyingIterator = typename Super::Iterator;

		template<typename... Arguments>
		UnderlyingIterator AddInternal(Entity entity, bool forceAddAtEnd, Arguments&&... arguments)
		{
			auto iterator = Super::AddInternal(entity, forceAddAtEnd);
			ComponentType* component = GetOrCreateComponentPointer(iterator.Index());
			new(component) ComponentType(std::forward<Arguments>(arguments)...);

			return iterator;
		}

		void EraseInternal(Entity entity) override
		{
			const u64 packedIndex = Super::GetPackedIndex(entity);
			ComponentType& component = GetComponentReference(packedIndex);

			if constexpr(InPlaceDelete)
			{
				Super::EraseInternal(entity);
				component.~ComponentType();
			}
			else // SwapAndErase
			{
				ComponentType& otherComponent = GetComponentReference(Super::Size() - 1);
				component = std::move(otherComponent);
				otherComponent.~ComponentType();

				Super::EraseInternal(entity);
			}
		}

		void MoveOrSwapPayload(u64 fromPackedIndex, u64 toPackedIndex)
		{
			constexpr bool isMovable = std::is_move_constructible_v<ComponentType> && std::is_move_assignable_v<ComponentType>;
			B3D_ASSERT(isMovable);

			if constexpr(isMovable)
			{
				if constexpr(InPlaceDelete)
				{
					if(Super::operator[](toPackedIndex) == kInvalidEntity)
					{
						ComponentType& componentToMove = GetComponentReference(fromPackedIndex);

						ComponentType* destinationComponent = GetOrCreateComponentPointer(toPackedIndex);
						new(destinationComponent) ComponentType(std::move(componentToMove));

						componentToMove.~ComponentType();
					}
					else
						std::swap(GetComponentReference(fromPackedIndex), GetComponentReference(toPackedIndex));
				}
				else
					std::swap(GetComponentReference(fromPackedIndex), GetComponentReference(toPackedIndex));
			}
		}

		void ShrinkComponentArray(u64 newComponentCount)
		{
			const u64 oldComponentCount = Super::Size();
			for(u64 packedComponentIndex = newComponentCount; packedComponentIndex < oldComponentCount; ++packedComponentIndex)
			{
				if constexpr(InPlaceDelete)
				{
					if(Super::mPackedEntities[packedComponentIndex] != kInvalidEntity)
						GetComponentReference(packedComponentIndex).~ComponentType();
				}
				else
					GetComponentReference(packedComponentIndex).~ComponentType();
			}

			const auto newPageCount = (newComponentCount + PackedPageSize - 1u) / PackedPageSize;
			const u64 oldPageCount = mComponents.Size();
			for(u64 page = newPageCount; page < oldPageCount; ++page)
				B3DFree(mComponents[page].Data());

			mComponents.Resize(newPageCount);
			mComponents.Shrink();
		}

		ComponentType* GetOrCreateComponentPageFor(u64 packedComponentIndex)
		{
			const u64 page = GetComponentPage(packedComponentIndex);

			if(page >= mComponents.Size())
				mComponents.Resize(page + 1);

			if(mComponents[page].IsEmpty())
				mComponents[page] = TArrayView(B3DAllocateMultiple<ComponentType>(PackedPageSize), PackedPageSize);

			return mComponents[page].Data();
		}

		ComponentType* GetOrCreateComponentPointer(u64 packedComponentIndex)
		{
			return GetOrCreateComponentPageFor(packedComponentIndex) + GetComponentIndexWithinPage(packedComponentIndex);
		}

		ComponentType& GetComponentReference(u64 packedComponentIndex)
		{
			return mComponents[GetComponentPage(packedComponentIndex)][GetComponentIndexWithinPage(packedComponentIndex)];
		}

		const ComponentType& GetComponentReference(u64 packedComponentIndex) const
		{
			return mComponents[GetComponentPage(packedComponentIndex)][GetComponentIndexWithinPage(packedComponentIndex)];
		}

		static constexpr u64 GetComponentIndexWithinPage(u64 packedComponentIndex)
		{
			return packedComponentIndex & (PackedPageSize - 1);
		}

		static constexpr u64 GetComponentPage(u64 packedComponentIndex)
		{
			return packedComponentIndex / PackedPageSize;
		}

		ComponentContainerType mComponents;
	};

	template<typename TagType>
	class TTagSparseSet : public TSparseSet<SparseSetDeletePolicy::SwapAndErase>
	{
	public:
		using ElementType = TagType;
		using Super = TSparseSet<SparseSetDeletePolicy::SwapAndErase>;

		~TTagSparseSet() override = default;

		void Add(Entity entity)
		{
			Super::AddInternal(entity, false);
		}

		template<typename It>
		void Add(It first, It last)
		{
			for(It it = first; it != last; ++it)
				Super::AddInternal(*it, true);
		}
	};

	class EntitySparseSet : public TSparseSet<SparseSetDeletePolicy::SwapOnly>
	{
	public:
		using ElementType = Entity;
		using Super = TSparseSet<SparseSetDeletePolicy::SwapOnly>;

		~EntitySparseSet() override = default;

		Entity Create()
		{
			Entity entity = Size() == GetFreeListHead() ? CreateEntity() : mPackedEntities[GetFreeListHead()];
			return *Super::AddInternal(entity, false);
		}

		Entity Create(Entity hint)
		{
			if(hint != kInvalidEntity && hint != kNullEntity)
			{
				Entity entity(hint.GetIdentifier(), GetVersion(hint));
				if(entity == kInvalidEntity || GetPackedIndex(entity) >= GetFreeListHead())
					return *Super::AddInternal(entity, false);
			}

			return Create();
		}

		void Clear() override
		{
			Super::Clear();
			mNextEntityId = 0u;
		}

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

	template<typename Type, typename = void>
	struct StorageForType;

	template<typename Type>
	struct StorageForType<Type, std::enable_if_t<std::conjunction_v<std::is_move_constructible<std::remove_const_t<Type>>, std::is_move_assignable<std::remove_const_t<Type>>, std::negation<std::is_empty<Type>>>>>
	{
		using StorageType = TComponentSparseSet<std::remove_const_t<Type>>;
	};

	template<typename Type>
	struct StorageForType<Type, std::enable_if_t<std::conjunction_v<std::disjunction<std::negation<std::is_move_constructible<std::remove_const_t<Type>>>, std::negation<std::is_move_assignable<std::remove_const_t<Type>>>>, std::negation<std::is_empty<Type>>>>>
	{
		using StorageType = TComponentSparseSet<std::remove_const_t<Type>, true>;
	};

	template<typename Type>
	struct StorageForType<Type, std::enable_if_t<std::is_empty_v<Type>>>
	{
		using StorageType = TTagSparseSet<Type>;
	};

	template<>
	struct StorageForType<Entity>
	{
		using StorageType = EntitySparseSet;
	};

	template<typename Type>
	using TStorageType = typename StorageForType<Type>::StorageType;

	template<typename It>
	static bool IsEntityPartOfAll(It first, It last, Entity entity)
	{
		while(first != last && (*first)->Contains(entity))
			++first;

		return first == last;
	}

	template<typename It>
	static bool IsEntityPartOfAny(It first, It last, Entity entity)
	{
		while(first != last && !(*first)->Contains(entity))
			++first;

		return first != last;
	}

	template<u32 IncludedTypeCount, u32 ExcludedTypeCount, bool StorageMayContainInvalidEntities>
	static bool DoesEntityMatchFilter(Entity entity, const std::array<const SparseSet*, IncludedTypeCount>& includedTypeStorage, const std::array<const SparseSet*, ExcludedTypeCount>& excludedTypeStorage, u32 leadingTypeIndex)
	{
		// Must not be deleted entity (only relevant for in-place deletion), must be contained in all the included type storages, and must not be part of any excluded type storages
		return (!StorageMayContainInvalidEntities || entity != kInvalidEntity)
			&& ((IncludedTypeCount == 1u) || (IsEntityPartOfAll(includedTypeStorage.begin(), includedTypeStorage.begin() + leadingTypeIndex, entity) && IsEntityPartOfAll(includedTypeStorage.begin() + leadingTypeIndex + 1, includedTypeStorage.end(), entity)))
			&& ((ExcludedTypeCount == 0u) || (!IsEntityPartOfAny(excludedTypeStorage.begin(), excludedTypeStorage.end(), entity)));
	}

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

		friend constexpr bool operator==(const TViewIterator& lhs, const TViewIterator& rhs) noexcept;
		friend constexpr bool operator!=(const TViewIterator& lhs, const TViewIterator& rhs) noexcept;

	private:
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

	template<u32 IncludedTypeCount, u32 ExcludedTypeCount, bool InPlaceDelete>
	class TViewCommon
	{
	public:
		using Iterator = TViewIterator<IncludedTypeCount, ExcludedTypeCount, InPlaceDelete>;

		const SparseSet* GetLeadingTypeStorage() const
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return nullptr;

			return mIncludedTypeStorage[mLeadingTypeIndex];
		}

		u64 GetSizeEstimate() const
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return 0u;

			return GetLeadingTypeStorageSize();
		}

		Iterator Begin() const
		{
			return mLeadingTypeIndex != IncludedTypeCount ? Iterator(mIncludedTypeStorage[mLeadingTypeIndex]->Begin(), mIncludedTypeStorage, mExcludedTypeStorage, mLeadingTypeIndex) : Iterator();
		}

		Iterator End() const
		{
			return mLeadingTypeIndex != IncludedTypeCount ? Iterator(mIncludedTypeStorage[mLeadingTypeIndex]->Begin() + GetLeadingTypeStorageSize(), mIncludedTypeStorage, mExcludedTypeStorage, mLeadingTypeIndex) : Iterator();
		}

		Entity Front() const
		{
			auto it = Begin();
			return  it != End() ? *it : kNullEntity;
		}

		Entity Back() const
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return kNullEntity;

			auto it = mIncludedTypeStorage[mLeadingTypeIndex]->Rbegin();
			const auto last = it + GetLeadingTypeStorageSize();

			while (it != last && !DoesEntityMatchViewFilter(*it))
				++it;

			return it != last() ? *it : kNullEntity;
		}

		Iterator Find(Entity entity)
		{
			if(Contains(entity))
				return Iterator(mIncludedTypeStorage[mLeadingTypeIndex]->Find(entity), mIncludedTypeStorage, mExcludedTypeStorage, mLeadingTypeIndex);

			return End();
		}

		bool Contains(Entity entity)
		{
			if(mLeadingTypeIndex == IncludedTypeCount)
				return false;

			return IsEntityPartOfAll(mIncludedTypeStorage.begin(), mIncludedTypeStorage.end(), entity)
				&& !IsEntityPartOfAny(mExcludedTypeStorage.begin(), mExcludedTypeStorage.end(), entity)
				&& (mIncludedTypeStorage[mLeadingTypeIndex]->GetPackedIndex(entity) < GetLeadingTypeStorageSize());
		}

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
		TViewCommon()
		{
			for(auto& entry : mExcludedTypeStorage)
				entry = GetPlaceholderStorage();
		}

		TViewCommon(std::array<const SparseSet*, IncludedTypeCount>& includedTypeStorage, std::array<const SparseSet*, ExcludedTypeCount> excludedTypeStorage)
			:mIncludedTypeStorage(includedTypeStorage), mExcludedTypeStorage(excludedTypeStorage), mLeadingTypeIndex(IncludedTypeCount)
		{
			RefreshLeadingTypeIndex();
		}

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

		const SparseSet* GetIncludedTypeStorage(u32 index) const
		{
			return mIncludedTypeStorage[index];
		}

		void SetIncludedTypeStorage(u32 index, const SparseSet* storage)
		{
			B3D_ASSERT(storage != nullptr);

			mIncludedTypeStorage[index] = storage;
			RefreshLeadingTypeIndexIfNeeded();
		}

		const SparseSet* GetExcludedTypeStorage(u32 index) const
		{
			return mExcludedTypeStorage[index] == GetPlaceholderStorage() ? nullptr : mExcludedTypeStorage[index];
		}

		void SetExcludedTypeStorage(u32 index, const SparseSet* storage)
		{
			B3D_ASSERT(storage != nullptr);

			mExcludedTypeStorage[index] = storage;
		}

		void SetExplicitLeadingTypeIndex(u32 index)
		{
			// Must assign all storage types before doing this, as refreshing type index during storage
			// assignment relies on mLeadingTypeIndex == IncludedTypeCount until all storage is assigned
			B3D_ASSERT(mLeadingTypeIndex != IncludedTypeCount);

			mLeadingTypeIndex = index;
		}

		u64 GetLeadingTypeStorageSize() const
		{
			B3D_ASSERT(mLeadingTypeIndex != IncludedTypeCount);
			return mIncludedTypeStorage[mLeadingTypeIndex]->GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly ? mIncludedTypeStorage[mLeadingTypeIndex]->GetFreeListHead() : mIncludedTypeStorage[mLeadingTypeIndex]->Size();
		}

		static const SparseSet* GetPlaceholderStorage()
		{
			static const SparseSet kPlaceholder;
			return &kPlaceholder;
		}

	private:
		std::array<const SparseSet*, IncludedTypeCount> mIncludedTypeStorage;
		std::array<const SparseSet*, ExcludedTypeCount> mExcludedTypeStorage;
		u32 mLeadingTypeIndex = 0;
	};

	// TODO - Move to utility header
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

	template<typename Type, typename TypeList>
	constexpr bool TTypeListIndexOf = TTypeListIndexOfHelper<Type, TypeList>::Value;

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

	template<u32 Index, typename List>
	using TTypeListElementAt = typename TTypeListElementAtHelper<Index, List>::Type;


	template<typename... Types>
	struct TIncludedTypes : TTypeList<Types...>
	{
		explicit constexpr TIncludedTypes() = default;
	};

	template<typename... Types>
	struct TExcludedTypes : TTypeList<Types...>
	{
		explicit constexpr TExcludedTypes() = default;
	};

	template<typename T, typename From>
	struct TInheritConstFromHelper
	{
		using Type = std::remove_const_t<T>;
	};

	template<typename T, typename From>
	struct TInheritConstFromHelper<T, const From>
	{
		using Type = const T;
	};

	template<typename T, typename From>
	using TInheritConstFrom = typename TInheritConstFromHelper<T, From>::Type;

	template<typename... StorageType>
	static constexpr bool TAllTypesUseInPlaceDelete = ((sizeof...(StorageType) == 1u) && ... && (StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace));

	template<typename, typename, typename = void>
	class TView;

	template<typename... IncludedStorageType, typename... ExcludedStorageType>
	class TView<TIncludedTypes<IncludedStorageType...>, TExcludedTypes<ExcludedStorageType...>, std::enable_if_t<(sizeof...(IncludedStorageType) != 0u)>> : public TViewCommon<sizeof...(IncludedStorageType), sizeof...(ExcludedStorageType), TAllTypesUseInPlaceDelete<IncludedStorageType...>>
	{
		using Super = TViewCommon<sizeof...(IncludedStorageType), sizeof...(ExcludedStorageType), TAllTypesUseInPlaceDelete<IncludedStorageType...>>;

		template<u32 Index>
		using TStorageTypeAt = TTypeListElementAt<Index, TTypeList<IncludedStorageType..., ExcludedStorageType...>>;

		template<typename ElementType>
		static constexpr bool TIndexOfElementType = TTypeListIndexOf<std::remove_const_t<ElementType>, TTypeList<typename IncludedStorageType::ElementType..., typename ExcludedStorageType::ElementType...>>;
		
	public:
		TView() = default;
		TView(IncludedStorageType... includedType, ExcludedStorageType... excludedType)
			:Super::Super({includedType...}, {excludedType...})
		{ }

		TView(std::tuple<IncludedStorageType&...> includedTypes, std::tuple<ExcludedStorageType&...> excludedTypes)
			:TView(std::make_from_tuple<TView>(std::tuple_cat(includedTypes, excludedTypes)))
		{ }

		template<typename ElementType>
		void SetLeadingType()
		{
			SetLeadingType<TIndexOfElementType<ElementType>>();
		}

		template<u32 Index>
		void SetLeadingType()
		{
			Super::SetExplicitLeadingTypeIndex(Index);
		}

		template<typename ElementType>
		auto* GetStorage() const
		{
			return GetStorage<TIndexOfElementType<ElementType>>();
		}

		template<u32 Index>
		auto* GetStorage() const
		{
			static_assert(Index < (sizeof...(IncludedStorageType) + sizeof...(ExcludedStorageType)), "Index out of range.");

			if constexpr(Index < sizeof...(IncludedStorageType))
				return static_cast<TStorageTypeAt<Index>*>(const_cast<TInheritConstFrom<SparseSet, TStorageTypeAt<Index>>*>(Super::GetIncludedTypeStorage(Index)));
			else
				return static_cast<TStorageTypeAt<Index>*>(const_cast<TInheritConstFrom<SparseSet, TStorageTypeAt<Index>>*>(Super::GetExcludedTypeStorage(Index - sizeof...(IncludedStorageType))));
		}

		template<typename StorageType>
		void SetStorage(StorageType& storage)
		{
			SetStorage<TIndexOfElementType<typename StorageType::ElementType>>(storage);
		}

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

		decltype(auto) operator[](Entity entity) const
		{
			return Get(entity);
		}

		template<typename ElementType, typename... OtherElementType>
		decltype(auto) Get(Entity entity) const
		{
			return Get<TIndexOfElementType<ElementType>, TIndexOfElementType<OtherElementType>...>(entity);
		}

		template<u32... Index>
		decltype(auto) Get(Entity entity) const
		{
			if constexpr(sizeof...(Index) == 0)
				return std::tuple_cat(std::forward_as_tuple(GetStorage<IncludedStorageType::ElementType>()->Get(entity))...);
			else if constexpr(sizeof...(Index) == 1)
				return (GetStorage<Index>()->Get(entity), ...);
			else
				return std::tuple_cat(std::forward_as_tuple(GetStorage<Index>()->Get(entity))...);
		}
	};

	// TODO - TView specialization for a single type


	class Registry
	{
	public:
		const SparseSet* TryGetStorage(typeid_t typeId) const
		{
			if(auto found = mComponentStorage.find(typeId); found != mComponentStorage.end())
				return found->second.get();

			return nullptr;
		}

		SparseSet* TryGetStorage(typeid_t typeId)
		{
			return const_cast<SparseSet*>(std::as_const(*this).TryGetStorage(typeId));
		}

		template<typename Type>
		const TStorageType<Type>* TryGetStorage() const
		{
			if constexpr(std::is_same_v<Type, Entity>)
				return static_cast<const TStorageType<Type>*>(&mEntityStorage);

			const typeid_t typeId = type_id<Type>();
			if(auto found = mComponentStorage.find(typeId); found != mComponentStorage.end())
				return static_cast<const TStorageType<Type>*>(found->second.get());

			return nullptr;
		}

		template<typename Type>
		TStorageType<Type>* TryGetStorage()
		{
			return const_cast<TStorageType<Type>*>(std::as_const(*this).TryGetStorage<Type>());
		}

		bool RemoveStorage(typeid_t typeId)
		{
			return mComponentStorage.erase(typeId) > 0;
		}

		template<typename Type>
		bool RemoveStorage()
		{
			const typeid_t typeId = type_id<Type>();
			return RemoveStorage(typeId);
		}

		template<typename... Type>
		void ClearStorage()
		{
			if(sizeof...(Type) == 0u)
				return;

			(GetOrCreateStorage<Type>().Clear(), ...);
		}

		void Clear()
		{
			for(auto& entry : mComponentStorage)
				entry.second->Clear();
			
			mEntityStorage.Clear();
		}

		bool IsEntityValid(Entity entity) const
		{
			if(auto found = mEntityStorage.Find(entity); found != mEntityStorage.End())
				return found.Index() < mEntityStorage.GetFreeListHead();

			return false;
		}

		bool HasEntityAnyComponents(Entity entity) const
		{
			for(auto& entry : mComponentStorage)
			{
				if(entry.second->Contains(entity))
					return true;
			}

			return false;
		}

		Entity::VersionType GetEntityVersion(Entity entity) const
		{
			return mEntityStorage.GetVersion(entity);
		}

		Entity CreateEntity()
		{
			return mEntityStorage.Create();
		}

		Entity CreateEntity(Entity hint)
		{
			return mEntityStorage.Create(hint);
		}

		Entity::VersionType DestroyEntity(Entity entity) // TODO - Inconsistent naming erase vs. delete
		{
			for(auto& entry : mComponentStorage)
				entry.second->EraseIfValid(entity);

			mEntityStorage.EraseIfValid(entity);
			return mEntityStorage.GetVersion(entity);
		}

		Entity::VersionType DestroyEntity(Entity entity, Entity::VersionType newVersion)
		{
			DestroyEntity(entity);

			Entity destroyedEntity(entity.GetIdentifier(), newVersion);
			if(destroyedEntity == kInvalidEntity)
				destroyedEntity = destroyedEntity.GetAsNextVersion();

			mEntityStorage.UpdateVersion(destroyedEntity);
			return destroyedEntity.GetVersion();
		}

		template<typename It>
		void DestroyEntities(It first, It last)
		{
			// Note: Deleting from the end would be more efficient. Perhaps in the future.
			const auto from = mEntityStorage.Begin();
			const auto to = mEntityStorage.SortAs(first, last);

			for(auto& entry : mComponentStorage)
				entry.second->EraseIfValid(from, to);

			mEntityStorage.Erase(from, to);
		}

		template<typename Type, typename... Arguments>
		Type& AddComponent(Entity entity, Arguments&&... arguments)
		{
			return GetOrCreateStorage<Type>().Add(entity, std::forward<Arguments>(arguments)...);
		}

		template<typename Type, typename It>
		void AddComponents(It first, It last, const Type& component = {})
		{
			B3D_ASSERT(std::all_of(first, last, [this](Entity entity) { return IsEntityValid(entity); }));
			GetOrCreateStorage<Type>().Add(std::move(first), std::move(last), component);
		}

		template<typename Type, typename... Arguments>
		Type& AddOrReplaceComponent(Entity entity, Arguments&&... arguments)
		{
			auto storage = GetOrCreateStorage<Type>();
			if(storage.Contains(entity))
			{
				Type& component = storage.Get(entity);
				component = Type{std::forward<Arguments>(arguments)...};
				return component;
			}

			return storage.template Add<Type>(entity, std::forward<Arguments>(arguments)...);
		}

		template<typename FirstComponentType, typename... OtherComponentType>
		u64 RemoveComponents(Entity entity)
		{
			return (GetOrCreateStorage<FirstComponentType>().EraseIfValid(entity) + ... + GetOrCreateStorage<OtherComponentType>(entity));
		}

		template<typename FirstComponentType, typename... OtherComponentType, typename It>
		u64 RemoveComponents(It first, It last)
		{
			u64 count = 0;
			auto relevantComponentStorageTuple = std::forward_as_tuple(GetOrCreateStorage<FirstComponentType>(), GetOrCreateStorage<OtherComponentType>()...);
			for(; first != last; ++first)
				count = std::apply([entity = *first](auto&... storage) { return (storage.EraseIfValid(entity) + ... + 0u); }, relevantComponentStorageTuple);

			return count;
		}

		template<typename... ComponentType>
		decltype(auto) GetComponents(Entity entity) const
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

		template<typename... ComponentType>
		decltype(auto) GetComponents(Entity entity)
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

		template<typename... ComponentType>
		decltype(auto) TryGetComponents(Entity entity) const
		{
			if constexpr(sizeof...(ComponentType) == 1u)
			{
				const auto& storage = TryGetStorage<std::remove_const_t<ComponentType>...>();
				return (storage != nullptr && storage.Contains(entity) ? &storage.Get(entity) : nullptr);
			}
			else
				return std::forward_as_tuple(TryGetComponents<ComponentType>(entity)...);
		}

		template<typename... ComponentType>
		decltype(auto) TryGetComponents(Entity entity)
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

		template<typename Type, typename... Arguments>
		Type& GetOrAddComponent(Entity entity, Arguments&&... arguments)
		{
			auto& storage = GetOrCreateStorage<Type>();
			return storage.Contains(entity) ? storage.Get(entity) : storage.Add(entity, std::forward<Arguments>(arguments)...);
		}

		template<typename FirstIncludedType, typename... OtherIncludedType, typename... ExcludedType>
		TView<TIncludedTypes<TStorageType<const FirstIncludedType>, TStorageType<const OtherIncludedType>...>, TExcludedTypes<TStorageType<const ExcludedType>...>>
		CreateView(TExcludedTypes<ExcludedType...> = TExcludedTypes<ExcludedType...>{}) const
		{
			TView<TIncludedTypes<TStorageType<const FirstIncludedType>, TStorageType<const OtherIncludedType>...>, TExcludedTypes<TStorageType<const ExcludedType>...>> view;

			[&view](const auto*... storage)
			{
				((storage != nullptr ? view.SetStorage(*storage) : void()), ...);
			}(TryGetStorage<std::remove_const_t<FirstIncludedType>>(), TryGetStorage<std::remove_const_t<OtherIncludedType>>()..., TryGetStorage<std::remove_const_t<ExcludedType>>()...);

			return view;
		}

		template<typename FirstIncludedType, typename... OtherIncludedType, typename... ExcludedType>
		TView<TIncludedTypes<TStorageType<FirstIncludedType>, TStorageType<OtherIncludedType>...>, TExcludedTypes<TStorageType<ExcludedType>...>>
		CreateView(TExcludedTypes<ExcludedType...> = TExcludedTypes<ExcludedType...>{}) 
		{
			TView<TIncludedTypes<TStorageType<FirstIncludedType>, TStorageType<OtherIncludedType>...>, TExcludedTypes<TStorageType<ExcludedType>...>> view;

			view.SetStorage(GetOrCreateStorage<std::remove_const_t<FirstIncludedType>>());
			(view.SetStorage(GetOrCreateStorage<std::remove_const_t<OtherIncludedType>>()), ...);
			(view.SetStorage(GetOrCreateStorage<std::remove_const_t<ExcludedType>>()), ...);

			return view;
		}

		template<typename Type, typename ComparisonFunction = std::less<>>
		void Sort()
		{
			GetOrCreateStorage<Type>().template Sort<ComparisonFunction>();
		}

		template<typename TypeToSort, typename TypeToSortAs>
		void SortAs()
		{
			const SparseSet& sortAsStorage = GetOrCreateStorage<TypeToSortAs>();
			GetOrCreateStorage<TypeToSort>().SortAs(sortAsStorage.Begin(), sortAsStorage.End());
		}

		template<typename... Type>
		void Shrink()
		{
			if constexpr(sizeof...(Type) == 0u)
			{
				for(auto&& storage : mComponentStorage)
					storage.second->Shrink();
			}
			else
			{
				(GetOrCreateStorage<Type>().Shrink(), ...);
			}
		}

		template<typename... ComponentType>
		bool HasAllOf(Entity entity) const
		{
			if constexpr(sizeof...(ComponentType) == 1u)
			{
				auto* storage = TryGetStorage<std::remove_const_t<ComponentType>...>();
				return storage != nullptr && storage->Contains(entity);
			}
			else
				return (HasAllOf<ComponentType>(entity) && ...);
		}

		template<typename... ComponentType>
		bool HasAnyOf(Entity entity) const
		{
			return (HasAllOf<ComponentType>(entity) || ...);
		}

	private:
		template<typename Type>
		TStorageType<Type>& GetOrCreateStorage()
		{
			if constexpr(std::is_same_v<Type, Entity>)
				return static_cast<TStorageType<Type>&>(mEntityStorage);

			const typeid_t typeId = type_id<Type>();
			if(auto found = mComponentStorage.find(typeId); found != mComponentStorage.end())
				return static_cast<TStorageType<Type>&>(*found->second);

			SPtr<SparseSet> componentStorage;
			if constexpr(std::is_empty_v<Type>)
				componentStorage = B3DMakeShared<TTagSparseSet<Type>>();
			else
			{
				static constexpr bool isTypeMovable = std::is_move_constructible_v<Type> && std::is_move_assignable_v<Type>;
				componentStorage = B3DMakeShared<TComponentSparseSet<Type, !isTypeMovable>>();
			}

			mComponentStorage[typeId] = componentStorage;
			return static_cast<TStorageType<Type>&>(*componentStorage);
		}

		EntitySparseSet mEntityStorage;
		UnorderedMap<typeid_t, SPtr<SparseSet>> mComponentStorage;

	};

	/** @} */
} // namespace bs
