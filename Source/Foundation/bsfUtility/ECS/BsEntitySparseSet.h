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
	template<typename>
	struct TEntityTypeTraits;

	template<>
	struct TEntityTypeTraits<u32>
	{
		using StorageType = u32;
		using IdentifierType = u32;
		using VersionType = u16;

		static constexpr StorageType kIdentifierMask = 0xFFFFF;
		static constexpr StorageType kVersionMask = 0xFFF;
	};

	template<>
	struct TEntityTypeTraits<u64>
	{
		using StorageType = u64;
		using IdentifierType = u64;
		using VersionType = u32;

		static constexpr StorageType kIdentifierMask = 0xFFFFFFFF;
		static constexpr StorageType kVersionMask = 0xFFFFFFFF;
	};

	template<typename Type>
	struct TEntity
	{
		using Traits = TEntityTypeTraits<Type>;
		using IdentifierType = typename Traits::IdentifierType;
		using VersionType = typename Traits::VersionType;

		static constexpr u32 kIdentifierMaskBitCount = CountBits(Traits::kIdentifierMask);

		constexpr TEntity() = default;
		constexpr TEntity(IdentifierType identifier, VersionType version)
		{
			if constexpr(Traits::kVersionMask == 0u)
				CombinedIdentifierAndVersion = (Type)(identifier & Traits::kIdentifierMask);
			else
				CombinedIdentifierAndVersion = ((Type)(identifier & Traits::kIdentifierMask)) | (((Type)(version & Traits::kVersionMask)) << kIdentifierMaskBitCount);
		}

		constexpr IdentifierType GetIdentifier() const
		{
			return (IdentifierType)(CombinedIdentifierAndVersion) & Traits::kIdentifierMask;
		}

		constexpr VersionType GetVersion() const
		{
			if constexpr(Traits::kVersionMask == 0u)
				return VersionType{};

			return (Traits::VersionType)(CombinedIdentifierAndVersion >> kIdentifierMaskBitCount) & Traits::kVersionMask;
		}

		constexpr TEntity GetAsNextVersion() const
		{
			return TEntity(GetIdentifier(), GetVersion() + 1);
		}
		
		typename Traits::StorageType CombinedIdentifierAndVersion{};
	};

#define B3D_ECS_64BIT_ENTITIES 0

#if B3D_ECS_64BIT_ENTITIES
	using Entity = TEntity<u64>;
#else
	using Entity = TEntity<u32>;
#endif




	//// TODO - Doc
	//template<typename Type>
	//class TEntitySparseSetTraits
	//{
	//	using EntityTypeTraits = TEntityTypeTraits<Type>;

	//public:
	//	using EntityType = typename EntityTypeTraits::IdentifierType;
	//	using VersionType = typename EntityTypeTraits::VersionType;

	//	static constexpr EntityType kEntityMask = EntityTypeTraits::ValueMask;
	//	static constexpr EntityType kVersionMask = EntityTypeTraits::VersionMask;
	//	static constexpr u32 kEntityMaskBitCount = CountBits(EntityTypeTraits::EntityMask);

	//	static constexpr EntityType ToEntity(Type value)
	//	{
	//		return static_cast<EntityType>(value) & kEntityMask;
	//	}

	//	static constexpr VersionType ToVersion(Type value)
	//	{
	//		if constexpr(kVersionMask == 0u)
	//			return VersionType{};

	//		return (VersionType)(value >> kEntityMaskBitCount) & kVersionMask;
	//	}

	//	static constexpr Type BumpVersion(const Type value)
	//	{
	//		const VersionType nextVersion = ToVersion(value) + 1;
	//		return PackEntityAndVersion(value, (VersionType)(nextVersion + (nextVersion == kVersionMask)));
	//	}

	//	static constexpr Type PackEntityAndVersion(EntityType value, VersionType version)
	//	{
	//		if constexpr(EntityTypeTraits::kVersionMask == 0u)
	//			return StorageType(value & kEntityMask);

	//		return StorageType((value & kEntityMask) | ((EntityType)(version & kVersionMask) << kEntityMaskBitCount));
	//	}
	//};

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
	struct TEntitySparseSetIterator final
	{
		using value_type = typename ContainerType::value_type;
		using pointer = typename ContainerType::const_pointer;
		using reference = typename ContainerType::const_reference;
		using difference_type = typename ContainerType::difference_type;
		using const_pointer = typename ContainerType::const_pointer;
		using const_reference = typename ContainerType::const_reference;
		using iterator_category = std::random_access_iterator_tag;

		TEntitySparseSetIterator() = default;
		TEntitySparseSetIterator(const ContainerType& container, i64 index)
			: mContainer(&container), mIndex(index) { }

		TEntitySparseSetIterator& operator++()
		{
			++mIndex;
			return *this;
		}

		TEntitySparseSetIterator& operator--()
		{
			--mIndex;
			return *this;
		}

		TEntitySparseSetIterator& operator+=(i64 value)
		{
			mIndex += value;
			return *this;
		}

		TEntitySparseSetIterator& operator-=(i64 value)
		{
			mIndex -= value;
			return *this;
		}

		TEntitySparseSetIterator operator+(i64 value) const
		{
			TEntitySparseSetIterator copy = *this;
			return (copy += value);
		}

		TEntitySparseSetIterator operator-(i64 value) const
		{
			return (*this + -value);
		}

		const_reference operator[](i64 value) const { return (*mContainer)[mIndex + value]; }
		const_pointer operator->() const { return std::addressof(operator[](0)); }
		const_reference operator*() const { return operator[](0); }

		i64 Index() const { return mIndex; }

	private:
		const ContainerType* mContainer = nullptr;
		i64 mIndex = 0;
	};

	template<typename ContainerType>
	i64 operator-(const TEntitySparseSetIterator<ContainerType>& lhs, const TEntitySparseSetIterator<ContainerType>& rhs)
	{
		return rhs.Index() - lhs.Index();
	}

	template<typename ContainerType>
	bool operator==(const TEntitySparseSetIterator<ContainerType>& lhs, const TEntitySparseSetIterator<ContainerType>& rhs)
	{
		return lhs.Index() == rhs.Index();
	}

	template<typename ContainerType>
	bool operator!=(const TEntitySparseSetIterator<ContainerType>& lhs, const TEntitySparseSetIterator<ContainerType>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename ContainerType>
	bool operator<(const TEntitySparseSetIterator<ContainerType>& lhs, const TEntitySparseSetIterator<ContainerType>& rhs)
	{
		return lhs.index() > rhs.index();
	}

	template<typename ContainerType>
	bool operator>(const TEntitySparseSetIterator<ContainerType>& lhs, const TEntitySparseSetIterator<ContainerType>& rhs)
	{
		return rhs < lhs;
	}

	template<typename ContainerType>
	bool operator<=(const TEntitySparseSetIterator<ContainerType>& lhs, const TEntitySparseSetIterator<ContainerType>& rhs)
	{
		return !(lhs > rhs);
	}

	template<typename ContainerType>
	bool operator>=(const TEntitySparseSetIterator<ContainerType>& lhs, const TEntitySparseSetIterator<ContainerType>& rhs)
	{
		return !(lhs < rhs);
	}

	// TODO - Doc
	enum class EntitySparseSetDeletePolicy
	{
		InPlace,
		SwapAndErase,
		SwapOnly
	};

	template<EntitySparseSetDeletePolicy DeletePolicy = EntitySparseSetDeletePolicy::SwapAndErase, u64 SparsePageSize = 4096>
	class TEntitySparseSet
	{
		using SparseContainerType = TArray<Entity*>;
		using PackedContainerType = TArray<Entity>;

		static_assert(SparsePageSize && (SparsePageSize & SparsePageSize - 1) == 0, "SparsePageSize must be a power of two value.");

		static constexpr u64 kMaximumEntryCount = Entity::Traits::kIdentifierMask;
	public:
		using Iterator = TEntitySparseSetIterator<PackedContainerType>;
		using ConstIterator = Iterator;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

		TEntitySparseSet() = default;
		~TEntitySparseSet()
		{
			FreeSparsePages();
		}

		Entity operator[](u64 index)
		{
			return mPacked[index];
		}

		void Add(Entity entity, bool forceAddAtEnd = false)
		{
			Entity& sparseSetEntry = GetOrCreateSparseEntryReference(entity);
			u64 packedEntryIndex = mPacked.Size();

			if constexpr(DeletePolicy == EntitySparseSetDeletePolicy::InPlace)
			{
				B3D_ENSURE(sparseSetEntry == kNullEntity);

				if(mFreeListHead != kMaximumEntryCount && !forceAddAtEnd)
				{
					packedEntryIndex = mFreeListHead;
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(mFreeListHead), entity.GetVersion());
					mFreeListHead = (u64)(std::exchange(mPacked[packedEntryIndex], entity).GetIdentifier());
				}
				else
				{
					mPacked.Add(entity);
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
				}
			}
			else if constexpr(DeletePolicy == EntitySparseSetDeletePolicy::SwapAndErase)
			{
				mPacked.Add(entity);
				B3D_ENSURE(sparseSetEntry == kNullEntity);
				sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
			}
			else
			{
				if(sparseSetEntry == kNullEntity)
				{
					mPacked.Add(entity);
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
				}
				else
				{
					B3D_ENSURE(!(sparseSetEntry.GetIdentifier() < mFreeListHead));
					sparseSetEntry = sparseSetEntry.GetAsNextVersion();
				}

				packedEntryIndex = mFreeListHead++;
				SwapEntriesAtPackedIndices(sparseSetEntry.GetIdentifier(), packedEntryIndex);
			}
		}

		void Erase(Iterator iterator)
		{
			const Entity entity = *iterator;
			if constexpr(DeletePolicy == EntitySparseSetDeletePolicy::InPlace)
			{
				// Sparse entry is set to a null value, while packed entry points to the next free packed entry, and its marked as invalid via its version
				const u64 packedEntryIndex = std::exchange(GetSparseEntryReference(entity), kNullEntity).GetIdentifier();
				mPacked[packedEntryIndex] = Entity(GetPackedIndexAsEntryIdentifier(std::exchange(mFreeListHead, packedEntryIndex)), ((Entity)kInvalidEntity).GetVersion());
			}
			else if constexpr(DeletePolicy == EntitySparseSetDeletePolicy::SwapAndErase)
			{
				// Set last sparse entry so it points to the packed index of the entry that was removed, swap packed entries
				Entity& sparseEntryToRemove = GetSparseEntryReference(entity);
				Entity& lastSparseEntry = GetSparseEntryReference(mPacked.Back());

				const u64 packedEntryToRemoveIndex = sparseEntryToRemove.GetIdentifier();
				lastSparseEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryToRemoveIndex), mPacked.Back().GetVersion());
				mPacked[packedEntryToRemoveIndex] = mPacked.Back();

				B3D_ENSURE(mPacked.Back() == kNullEntity);
				sparseEntryToRemove = kNullEntity;
				mPacked.Pop();
			}
			else
			{
				const u64 packedEntryIndex = GetPackedIndex(entity);
				UpdateVersion(entity.GetAsNextVersion());

				mFreeListHead -= (packedEntryIndex < mFreeListHead);
				SwapEntriesAtPackedIndices(packedEntryIndex, mFreeListHead);
			}
		}

		void Erase(Iterator first, Iterator last)
		{
			for(; first != last; ++first)
				Erase(first);
		}

		void Remove(Entity entity)
		{
			if(Contains(entity))
				Erase(GetIterator(entity));
		}

		void Clear()
		{
			//if constexpr(DeletePolicy == EntitySparseSetDeletePolicy::InPlace)
			//{
			//	if(mFreeListHead != kMaximumEntryCount)
			//	{
			//		for(auto&& entity : mPacked)
			//		{
			//			if(entity != kInvalidEntity)
			//				GetSparseEntryReference(entity) = kNullEntity;
			//		}
			//	}
			//	else
			//	{
			//		for(auto&& entity : mPacked)
			//			GetSparseEntryReference(entity) = kNullEntity;
			//	}
			//}
			//else
			//{
			//	for(auto&& entity : mPacked)
			//		GetSparseEntryReference(entity) = kNullEntity;
			//}

			FreeSparsePages();
			mPacked.Clear();
			mFreeListHead = DeletePolicy != EntitySparseSetDeletePolicy::SwapOnly ? kMaximumEntryCount : 0;
		}

		void ClearDeleted()
		{
			if constexpr(DeletePolicy != EntitySparseSetDeletePolicy::InPlace)
				return;

			u64 validPackedEntryIndex = mPacked.Size();
			u64 freePackedEntryIndex = std::exchange(mFreeListHead, kMaximumEntryCount);

			// Find first valid entry
			for(; validPackedEntryIndex > 0 && mPacked[validPackedEntryIndex - 1] == kInvalidEntity; --validPackedEntryIndex) { }

			while(freePackedEntryIndex != kMaximumEntryCount)
			{
				B3D_ENSURE(freePackedEntryIndex < validPackedEntryIndex);

				// Move the free entries to the back
				--validPackedEntryIndex;
				SwapEntriesAtPackedIndices(freePackedEntryIndex, validPackedEntryIndex);

				// Find next free entry
				freePackedEntryIndex = mPacked[freePackedEntryIndex];

				// Find next valid entry
				for(; validPackedEntryIndex > 0 && mPacked[validPackedEntryIndex - 1] == kInvalidEntity; --validPackedEntryIndex) { }
			}
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
			mPacked[packedEntryIndex] = entity;
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

		//u64 Size() const { return mPacked.Size(); } // Size is not valid for in-place deletion, it's just an estimate
		bool IsEmpty() const { return mPacked.Empty(); }

		Iterator Begin() const { return Iterator(mPacked, 0); }
		ConstIterator Cbegin() const { return Begin(); }

		Iterator End() const { return Iterator(mPacked, mPacked.Size()); }
		ConstIterator Cend() const { return End(); }

		ReverseIterator Rbegin() const { return std::make_reverse_iterator(end()); }
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

	private:
		void SwapEntriesAtPackedIndices(u64 lhsPackedIndex, u64 rhsPackedIndex)
		{
			Entity& lhsPackedEntry = mPacked[lhsPackedIndex];
			Entity& rhsPackedEntry = mPacked[rhsPackedIndex];

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

			if(sparsePage >= mSparse.Size())
				mSparse.Resize(sparsePage + 1);

			if(mSparse[sparsePage] == nullptr)
			{
				mSparse[sparsePage] = B3DAllocateMultiple<Entity>(SparsePageSize);
				std::uninitialized_fill(mSparse[sparsePage], mSparse[sparsePage] + SparsePageSize, kNullEntity);
			}

			return *(mSparse[sparsePage] + GetSparseIndexWithinPage(entityIdentifier));
		}

		Entity& GetSparseEntryReference(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			return mSparse[GetSparsePage(entityIdentifier)][GetSparseIndexWithinPage(entityIdentifier)];
		}

		Entity* GetSparseEntryPointer(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			const u64 sparsePage = GetSparsePage(entityIdentifier);

			if(sparsePage < mSparse.Size() && mSparse[sparsePage] != nullptr)
				return mSparse[sparsePage][GetSparseIndexWithinPage(entityIdentifier)];

			return nullptr;
		}

		Iterator GetIterator(Entity entity)
		{
			return Iterator(mPacked, (i64)GetPackedIndex(entity));
		}

		u64 GetPackedIndex(Entity entity) const
		{
			return GetSparseEntryReference(entity).GetIdentifier();
		}

		void FreeSparsePages()
		{
			for(auto&& page : mSparse)
			{
				if(page != nullptr)
				{
					std::destroy(page, page + SparsePageSize);
					B3DFree(page);
					page = nullptr;
				}
			}
			mSparse.Clear();
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

		SparseContainerType mSparse;
		PackedContainerType mPacked; // TODO - Might consider paging this. It won't be continous anymore, making it harder to iterate, but adding entries might prevent expensive resizes if there's a lot of entries

		/**
		 * For in-place delete policy points to first free entry, or kMaximumEntryCount if no free entries.
		 * For swap-only delete policy points to the first free entry, where all other elements are sequentially following the first free element. This value corresponds to valid entry count.
		 * For swap-and-erase delete policy this value is not used.
		 */
		u64 mFreeListHead = DeletePolicy != EntitySparseSetDeletePolicy::SwapOnly ? kMaximumEntryCount : 0;
	};

	/** @} */
} // namespace bs
