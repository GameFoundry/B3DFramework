//************************************ B3D Framework - Copyright 2018-2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Helper type that provides additional information about a key type used in DenseMap.
	 *
	 * Must provide the following methods:
	 *  - getEmptyKey() - Returns a value of Type that represents an empty spot in the hash-map. This value cannot be used
	 *					  for normal hash map entries.
	 *  - getTombstoneKey() - Returns a value of Type that represents a deleted spot in the hash-map. This value must be
	 *						  different from getEmptyKey() and cannot be used for normal hash map entries.
	 */
	template <class Type>
	struct DenseMapInfo;

	/** DenseMap iterator. */
	template <class Key, class Value, class KeyInfo = DenseMapInfo<Key>>
	class DenseMapIterator;

	/** DenseMap constant iterator. */
	template <class Key, class Value, class KeyInfo = DenseMapInfo<Key>>
	class DenseMapConstIterator;

	/**
	 * DenseMapInfo specialization that works well for unsigned integer types, but will be used for all other types
	 * you don't provide explicit specialization for.
	 */
	template <class Type>
	struct DenseMapInfo
	{
		constexpr static Type getEmptyKey() { return static_cast<Type>(-1); }

		constexpr static Type getTombstoneKey() { return static_cast<Type>(-2); }
	};

	/** DenseMapInfo specialization that works for pointer types. */
	template <class Type>
	struct DenseMapInfo<Type*>
	{
		constexpr static Type* getEmptyKey() { return reinterpret_cast<Type*>(-1); }

		constexpr static Type* getTombstoneKey() { return reinterpret_cast<Type*>(-2); }
	};

	/** DenseMap iterator implementation. */
	template <class Key, class Value, class KeyInfo>
	class DenseMapIterator
	{
	public:
		typedef std::pair<Key, Value> DensePair;

		DenseMapIterator(const DensePair* ptr, const DensePair* end)
			: ptr(ptr), end(end)
		{
			next();
		}

		std::pair<Key, Value>& operator*() const { return *const_cast<DensePair*>(ptr); }

		std::pair<Key, Value>* operator->() const { return const_cast<DensePair*>(ptr); }

		bool operator==(const DenseMapIterator& rhs) const { return ptr == rhs.ptr; }

		bool operator!=(const DenseMapIterator& rhs) const { return ptr != rhs.ptr; }

		DenseMapIterator& operator++()
		{
			++ptr;
			next();

			return *this;
		}

		DenseMapIterator operator++(int)
		{
			DenseMapIterator tmp = *this;
			++*this;

			return tmp;
		}

	protected:
		const DensePair* ptr;
		const DensePair* end;

	private:
		void next()
		{
			while(ptr != end && ((ptr->first == KeyInfo::getEmptyKey()) || (ptr->first == KeyInfo::getTombstoneKey())))
				++ptr;
		}
	};

	/** DenseMap constant iterator implementation. */
	template <class Key, class Value, class KeyInfo>
	class DenseMapConstIterator final : public DenseMapIterator<Key, Value, KeyInfo>
	{
	public:
		DenseMapConstIterator(const std::pair<Key, Value>* ptr, const std::pair<Key, Value>* end)
			: DenseMapIterator<Key, Value, KeyInfo>(ptr, end)
		{}

		const std::pair<Key, Value>& operator*() const { return *this->ptr; }

		const std::pair<Key, Value>* operator->() const { return this->ptr; }
	};

	/**
	 * Hash-map with densely stored values, using quadratic probing for lookup.
	 *
	 * @tparam Key		Type that represents the key in the map. Must have a specialization for DenseMapInfo<Type>,
	 *					that provides a value representing an empty key value and a tombstone key value
	 *					(@see DenseMapInfo<Type>). Also must have a @p B3DHash<T> specialization, and an equality
	 *					operator.
	 * @tparam	Value	Type of values to store in the hash-map.
	 * @tparam	KeyInfo	Type that provides information about empty and tombstone key values for the provided type. The
	 *					default value will handle pointers and unsigned integers, using -1 for empty key and -2 for
	 *					tombstone key value.
	 * @tparam	Size	Initial capacity of the map.
	 *
	 */
	template <class Key, class Value, class KeyInfo = DenseMapInfo<Key>, u32 Size = 64>
	class TDenseMap
	{
	public:
		typedef std::pair<Key, Value> DensePair;
		typedef DenseMapIterator<Key, Value, KeyInfo> Iterator;
		typedef DenseMapConstIterator<Key, Value, KeyInfo> ConstIterator;

		TDenseMap()
		{
			init();
		}

		TDenseMap(const TDenseMap<Key, Value>& other)
		{
			mCount = 0;
			if(!other.empty())
				copy(other);
		}

		TDenseMap(TDenseMap<Key, Value>&& other)
		{
			mCount = 0;
			if(!other.empty())
				copy(std::move(other));
		}

		TDenseMap(u32 n)
		{
			append(n);
		}

		~TDenseMap()
		{
			for(DensePair *pair = getBuckets(), *e = getBuckets() + getCount(); pair != e; ++pair)
			{
				if(!(pair->first == getEmptyKey()) && !(pair->first == getTombstoneKey()))
					pair->second.~Value();

				pair->first.~Key();
			}

			B3DFree(mBuckets);
		}

		Value& operator[](const Key& key)
		{
			return construct(key).second;
		}

		TDenseMap& operator=(const TDenseMap<Key, Value>& other)
		{
			if(this == other)
				return *this;

			copy(other);
			return *this;
		}

		bool operator==(const TDenseMap<Key, Value>& other) const
		{
			if(this->size() != other.size()) return false;
			return std::equal(this->begin(), this->end(), other.begin());
		}

		bool operator!=(const TDenseMap<Key, Value>& other) const
		{
			return !(*this == other);
		}

		Iterator begin()
		{
			return Iterator(mBuckets, mBuckets + mCount);
		}

		Iterator end()
		{
			return Iterator(mBuckets + mCount, mBuckets + mCount);
		}

		ConstIterator begin() const
		{
			return ConstIterator(mBuckets, mBuckets + mCount);
		}

		ConstIterator end() const
		{
			return ConstIterator(mBuckets + mCount, mBuckets + mCount);
		}

		bool empty() const { return mEntries == 0; }

		u32 size() const { return mEntries; }

		/** Grow so that it has at least n mBuckets. */
		void resize(u32 n)
		{
			grow(n);
		}

		void clear()
		{
			// If the capacity of the array is huge and the elements used is small then shrink the array.
			if(mEntries * 4 < mCount && mCount > Size)
			{
				shrink();
				return;
			}

			const Key emptyKey = getEmptyKey();
			for(DensePair *pair = getBuckets(), *e = getBuckets() + getCount(); pair != e; ++pair)
			{
				if(!(pair->first == emptyKey))
				{
					if(!(pair->first == getTombstoneKey()))
					{
						pair->second.~Value();
						--mEntries;
					}

					pair->first = emptyKey;
				}
			}

			B3D_ASSERT(size() == 0 && "The number of nodes is imbalance");
			mTombstones = 0;
		}

		bool contains(const Key& key) const
		{
			DensePair* temp;
			return lookup(key, temp);
		}

		Iterator find(const Key& key)
		{
			DensePair* temp;

			if(lookup(key, temp))
				return Iterator(temp, mBuckets + mCount);

			return end();
		}

		ConstIterator find(const Key& key) const
		{
			DensePair* temp;

			if(lookup(key, temp))
				return ConstIterator(temp, mBuckets + mCount);

			return end();
		}

		std::pair<Iterator, bool> insert(const std::pair<Key, Value>& pair)
		{
			if(empty())
				init();

			DensePair* temp;

			// It is already in map
			if(lookup(pair.first, temp))
				return std::make_pair(Iterator(temp, getBuckets() + getCount()), false);

			// Otherwise insert the new element
			temp = insertBucket(pair.first, pair.second, temp);

			return std::make_pair(Iterator(temp, getBuckets() + getCount()), true);
		}

		bool erase(const Key& key)
		{
			B3D_ASSERT(size() != 0);
			DensePair* temp;

			if(!lookup(key, temp))
				return false;

			update(temp);

			return true;
		}

		bool erase(Iterator iter)
		{
			B3D_ASSERT(size() != 0);
			DensePair* temp = &*iter;

			update(temp);

			return true;
		}

		DensePair& construct(const Key& key)
		{
			DensePair* temp;

			if(lookup(key, temp))
				return *temp;

			return *insertBucket(key, Value(), temp);
		}

	private:
		void init()
		{
			append(Size);
		}

		void update(DensePair* t)
		{
			t->second.~Value();
			t->first = getTombstoneKey();
			--mEntries;
			++mTombstones;
		}

		void copy(const TDenseMap<Key, Value>& other)
		{
			if(getCount() != 0 && (!std::is_pod<Key>() || !std::is_pod<Value>()))
			{
				const Key EmptyKey = getEmptyKey();
				const Key TombstoneKey = getTombstoneKey();
				for(DensePair *pair = getBuckets(), *e = getBuckets() + getCount(); pair != e; ++pair)
				{
					if(!(pair->first == EmptyKey) && !(pair->first == TombstoneKey))
						pair->second.~Value();

					pair->first.~Key();
				}
			}

			mEntries = other.size();
			mTombstones = other.getTombstones();

			if(getCount())
				B3DFree(mBuckets);

			mBuckets = B3DAllocateMultiple<DensePair>(other.getCount());

			if(std::is_pod<Key>() && std::is_pod<Value>())
				memcpy(getBuckets(), other.getBuckets(), other.getCount() * sizeof(DensePair));
			else
				for(u32 bucketIndex = 0; bucketIndex < other.getCount(); ++bucketIndex)
				{
					new(&mBuckets[bucketIndex].first) Key(other.mBuckets[bucketIndex].first);

					if(!(mBuckets[bucketIndex].first == getEmptyKey()) && !(mBuckets[bucketIndex].first == getTombstoneKey()))
						new(&mBuckets[bucketIndex].second) Value(other.mBuckets[bucketIndex].second);
				}

			mCount = other.getCount();
		}

		DensePair* insertBucket(const Key& key, const Value& value, DensePair* temp)
		{
			// If the hash table load is more than 3/4 or is it fewer than 1/8 of the buckets then grow the table.
			// If there is one empty bucket with tons of tombstones, missing lookups would have to
			// probe almost the entire table until it found the empty bucket. If the
			// table is completely filled with tombstones, no lookup would ever succeed, making init loops in lookup.
			if(size() * 4 >= getCount() * 3 || getCount() - (size() + getTombstones()) < getCount() / 8)
			{
				this->grow(getCount() * 2);
				lookup(key, temp);
			}

			++mEntries;

			// If we are writing over a tombstone, remember this
			if(!(temp->first == getEmptyKey()))
				--mTombstones;

			temp->first = key;
			new(&temp->second) Value(value);

			return temp;
		}

		/**
		 * Lookup the appropriate bucket for key, returning it in @p f. If the bucket contains the key and a value,
		 * this returns true, otherwise it returns a bucket with an empty marker or tombstone and returns false.
		 */
		bool lookup(const Key& key, DensePair*& f) const
		{
			u32 bucketN = getHashValue(key);
			u32 probe = 1;
			DensePair* bucket = getBuckets();

			// Keep track of whether we find a tombstone while probing.
			DensePair* foundTombstone = 0;

			const Key EmptyKey = getEmptyKey();
			const Key TombstoneKey = getTombstoneKey();
			B3D_ASSERT(!(key == EmptyKey) && !(key == TombstoneKey) && "Empty/Tombstone value shouldn't be inserted into map!");

			while(1)
			{
				DensePair* temp = bucket + (bucketN & (getCount() - 1));

				// If found the key bucket return it.
				if((temp->first == key))
				{
					f = temp;
					return true;
				}

				// If we found an empty bucket, the key doesn't exist in the set.
				// Insert it and return the default value.
				if((temp->first == EmptyKey))
				{
					// If we've already seen a tombstone while probing, fill it in instead
					// of the empty bucket we eventually probed to.
					if(foundTombstone)
						temp = foundTombstone;

					f = foundTombstone ? foundTombstone : temp;

					return false;
				}

				// If this is a tombstone, remember it.  If key ends up not in the map, we
				// prefer to return it than something that would require more probing.
				if((temp->first == TombstoneKey) && !foundTombstone)
					foundTombstone = temp;

				// Otherwise it's a hash collision or a tombstone, continue quadratic probing.
				bucketN += probe++;
			}
		}

		void append(u32 n)
		{
			mEntries = 0;
			mTombstones = 0;
			mCount = n;
			B3D_ASSERT(n && (n & (n - 1)) == 0 && "initial buckets must be a power of two!");

			mBuckets = B3DAllocateMultiple<DensePair>(n);

			for(u32 bucketIndex = 0; bucketIndex != n; ++bucketIndex)
				new(&mBuckets[bucketIndex].first) Key(getEmptyKey());
		}

		void grow(u32 n)
		{
			const u32 oldCount = getCount();
			DensePair* oldBuckets = getBuckets();

			// Double the number of buckets
			while(mCount <= n)
				mCount <<= 1;

			mTombstones = 0;
			mBuckets = B3DAllocateMultiple<DensePair>(getCount());

			// Initialize all the keys to EmptyKey
			const Key EmptyKey = getEmptyKey();
			for(u32 bucketIndex = 0, bucketCount = mCount; bucketIndex != bucketCount; ++bucketIndex)
				new(&mBuckets[bucketIndex].first) Key(EmptyKey);

			// Insert all the old elements
			const Key TombstoneKey = getTombstoneKey();
			for(DensePair *pair = oldBuckets, *e = oldBuckets + oldCount; pair != e; ++pair)
			{
				if(!(pair->first == EmptyKey) && !(pair->first == TombstoneKey))
				{
					// Insert the pair into the new table
					DensePair* destBucket;
					bool f = lookup(pair->first, destBucket);

					// silence warning
					f = f;
					B3D_ASSERT(!f && "Key already in new map");

					destBucket->first = pair->first;
					new(&destBucket->second) Value(pair->second);

					// Free the value
					pair->second.~Value();
				}

				pair->first.~Key();
			}
			// Free the old table
			B3DFree(oldBuckets);
		}

		void shrink()
		{
			const u32 oldCount = getCount();
			DensePair* oldBuckets = getBuckets();

			// Reduce the number of mBuckets
			mCount = size() > 32 ? 1 << ((int)std::log(size()) + 1) : 64;
			mTombstones = 0;
			mBuckets = B3DAllocateMultiple<DensePair>(getCount());

			// Initialize the keys to EmptyKey
			const Key EmptyKey = getEmptyKey();
			for(u32 bucketIndex = 0, bucketCount = mCount; bucketIndex != bucketCount; ++bucketIndex)
				new(&mBuckets[bucketIndex].first) Key(EmptyKey);

			// Free the old mBuckets
			const Key TombstoneKey = getTombstoneKey();
			for(DensePair *B = oldBuckets, *E = oldBuckets + oldCount; B != E; ++B)
			{
				if(!(B->first == EmptyKey) && !(B->first == TombstoneKey))
					B->second.~Value();

				B->first.~Key();
			}

			// Free the old mBuckets
			B3DFree(oldBuckets);
			mEntries = 0;
		}

		u32 getCount() const { return mCount; }

		DensePair* getBuckets() const { return mBuckets; }

		u32 getTombstones() const { return mTombstones; }

		static u32 getHashValue(const Key& key) { return (u32)B3DHash(key); }

		static const Key getEmptyKey() { return KeyInfo::getEmptyKey(); }

		static const Key getTombstoneKey() { return KeyInfo::getTombstoneKey(); }

	private:
		DensePair* mBuckets = nullptr;
		u32 mCount = 0;
		u32 mEntries = 0;
		u32 mTombstones = 0;
	};

	/** @} */
} // namespace b3d
