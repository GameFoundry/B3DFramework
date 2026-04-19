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

	/** Iterator that iterates over all entities that match the runtime view included & excluded type filter. */
	struct RuntimeViewIterator final
	{
	private:
		using UnderlyingIterator = SparseSet::Iterator;

	public:
		using value_type = std::iterator_traits<UnderlyingIterator>::value_type;
		using pointer = std::iterator_traits<UnderlyingIterator>::pointer;
		using reference = std::iterator_traits<UnderlyingIterator>::reference;
		using difference_type = std::iterator_traits<UnderlyingIterator>::difference_type;
		using iterator_category = std::bidirectional_iterator_tag;

		RuntimeViewIterator() = default;
		RuntimeViewIterator(UnderlyingIterator underlyingIterator, const TArray<SparseSet*>& includedTypeStorage, const TArray<SparseSet*>& excludedTypeStorage)
			: mUnderlyingIterator(underlyingIterator), mIncludedTypeStorage(&includedTypeStorage), mExcludedTypeStorage(&excludedTypeStorage)
			, mStorageMayContainInvalidEntries(includedTypeStorage.size() == 1 && includedTypeStorage[0]->GetDeletePolicy() == SparseSetDeletePolicy::InPlace)
		{
			SeekToNextValidEntry();
		}

		RuntimeViewIterator& operator++()
		{
			++mUnderlyingIterator;
			SeekToNextValidEntry();

			return *this;
		}

		RuntimeViewIterator operator++(int)
		{
			RuntimeViewIterator copy = *this;
			++(*this);
			return copy;
		}

		RuntimeViewIterator& operator--()
		{
			--mUnderlyingIterator;
			SeekToPreviousValidEntry();

			return *this;
		}

		RuntimeViewIterator operator--(int)
		{
			RuntimeViewIterator copy = *this;
			--(*this);
			return copy;
		}


		pointer operator->() const
		{
			return &*mUnderlyingIterator;
		}

		reference operator*() const
		{
			return *mUnderlyingIterator;
		}

		friend bool operator==(const RuntimeViewIterator& lhs, const RuntimeViewIterator& rhs);
		friend bool operator!=(const RuntimeViewIterator& lhs, const RuntimeViewIterator& rhs);

	private:
		/**
		 * Checks if entity matches the included & excluded type filters.
		 *
		 * @param	entity					Entity to check.
		 * @param	includedTypeStorage		Storages that must contain the entity to pass the filter.
		 * @param	excludedTypeStorage		Storages that must not contain the entity to pass the filter.
		 * @return
		 */
		bool DoesEntityMatchFilter(Entity entity, const TArray<SparseSet*>& includedTypeStorage, const TArray<SparseSet*>& excludedTypeStorage) const
		{
			// Must not be deleted entity (only relevant for in-place deletion), must be contained in all the included type storages, and must not be part of any excluded type storages
			return (!mStorageMayContainInvalidEntries || entity != kInvalidEntity)
				&& std::all_of(includedTypeStorage.begin() + 1, includedTypeStorage.end(), [entity](const auto* storage) { return storage->Contains(entity); })
				&& std::none_of(excludedTypeStorage.begin(), excludedTypeStorage.end(), [entity](const auto* storage) { return storage != nullptr && storage->Contains(entity); });
		}

		/**
		 * Increments the leading type storage to the next entry and checks if it matches the type filter. If it doesn't match the type
		 * filter the next entry is searched, and so on until the first matching entry or the end of leading type storage.
		 */
		void SeekToNextValidEntry()
		{
			// Iterate until the find next matching entity
			while(mUnderlyingIterator != (*mIncludedTypeStorage)[0]->End() && !DoesEntityMatchFilter(*mUnderlyingIterator, *mIncludedTypeStorage, *mExcludedTypeStorage))
				++mUnderlyingIterator;
		}

		/**
		 * Decrements the leading type storage to the previous entry and checks if it matches the type filter. If it doesn't match the type
		 * filter the next entry is searched, and so on until the first matching entry or the beginning of leading type storage.
		 */
		void SeekToPreviousValidEntry()
		{
			// Iterate until the find next matching entity
			while(mUnderlyingIterator != (*mIncludedTypeStorage)[0]->Begin() && !DoesEntityMatchFilter(*mUnderlyingIterator, *mIncludedTypeStorage, *mExcludedTypeStorage))
				--mUnderlyingIterator;
		}

		UnderlyingIterator mUnderlyingIterator;
		const TArray<SparseSet*>* mIncludedTypeStorage;
		const TArray<SparseSet*>* mExcludedTypeStorage;
		bool mStorageMayContainInvalidEntries = false;
	};

	bool operator==(const RuntimeViewIterator& lhs, const RuntimeViewIterator& rhs)
	{
		return lhs.mUnderlyingIterator == rhs.mUnderlyingIterator;
	}

	bool operator!=(const RuntimeViewIterator& lhs, const RuntimeViewIterator& rhs)
	{
		return !(lhs.mUnderlyingIterator == rhs.mUnderlyingIterator);
	}

	/** @} */

	/** @addtogroup ECS
	 *  @{
	 */

	/**
	 * Creates a view that allows you to iterate over all entities that match included & excluded type filter. Unlike TView,
	 * included and excluded type filters can be changed dynamically at runtime, rather than having to be known at compile time.
	 * However runtime views are less performant than regular views.
	 */
	class RuntimeView
	{
	public:
		using Iterator = RuntimeViewIterator;

		RuntimeView() = default;
		RuntimeView(const RuntimeView&) = default;
		RuntimeView(RuntimeView&&) = default;
		~RuntimeView() = default;

		RuntimeView& operator=(const RuntimeView&) = default;
		RuntimeView& operator=(RuntimeView&&) = default;

		/** Swaps internals between two views. */
		void Swap(RuntimeView& other)
		{
			std::swap(mIncludedTypeStorage, other.mIncludedTypeStorage);
			std::swap(mExcludedTypeStorage, other.mExcludedTypeStorage);
		}

		/** Adds a storage that the entity must be a part of in order to be included in the view. */
		void Include(SparseSet& storage)
		{
			if(mIncludedTypeStorage.Empty() || storage.Size() >= mIncludedTypeStorage[0]->Size())
				mIncludedTypeStorage.Add(&storage);
			else
				mIncludedTypeStorage.Add(std::exchange(mIncludedTypeStorage[0], &storage));
		}

		/** Adds a storage that the entity must not be a part of in order to be included in the view. */
		void Exclude(SparseSet& storage)
		{
			mExcludedTypeStorage.Add(&storage);
		}

		/** Clears all filters from the view. */
		void Clear()
		{
			mIncludedTypeStorage.Clear();
			mExcludedTypeStorage.Clear();
		}

		/** Returns a rough number of elements in the view, based on the number of elements in the leading storage. */
		u64 GetSizeEstimate() const
		{
			return mIncludedTypeStorage.Empty() ? 0 : mIncludedTypeStorage[0]->Size();
		}

		/** Iterator to the first entity matching the view filter. */
		Iterator Begin() const
		{
			return mIncludedTypeStorage.Empty() ? Iterator() : Iterator(mIncludedTypeStorage[0]->Begin(), mIncludedTypeStorage, mExcludedTypeStorage);
		}

		/** Iterator past the last entity matching the view filter. */
		Iterator End() const
		{
			return mIncludedTypeStorage.Empty() ? Iterator() : Iterator(mIncludedTypeStorage[0]->End(), mIncludedTypeStorage, mExcludedTypeStorage);
		}

		/** Returns true if the provided entity matches the view filter. */
		bool Contains(Entity entity) const
		{
			if(mIncludedTypeStorage.Empty())
				return false;

			return std::all_of(mIncludedTypeStorage.begin(), mIncludedTypeStorage.end(), [entity](const auto* storage) { return storage->Contains(entity); })
				&& std::none_of(mExcludedTypeStorage.begin(), mExcludedTypeStorage.end(), [entity](const auto* storage) { return storage != nullptr && storage->Contains(entity); });
		}

		/** Calls @p function for each entry in the view. Valid signature for @p function is void(Entity). */
		template<typename Function>
		void DoForEach(Function function) const
		{
			for(const Entity entity : *this)
				function(entity);
		}

		// For std compatibility
		using iterator = Iterator;

		iterator begin() const { return Begin(); }
		iterator end() const { return End(); }

	private:
		TArray<SparseSet*> mIncludedTypeStorage;
		TArray<SparseSet*> mExcludedTypeStorage;
	};

	/** @} */
} // namespace b3d::ecs
