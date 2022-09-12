//************************************ bs::framework - Copyright 2018-2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs 
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
	template<class Key, class Value,
		class KeyInfo = DenseMapInfo<Key>>
		class DenseMapIterator;

	/** DenseMap constant iterator. */
	template <class Key, class Value,
		class KeyInfo = DenseMapInfo<Key>>
		class DenseMapConstIterator;

	/** 
	 * DenseMapInfo specialization that works well for unsigned integer types, but will be used for all other types
	 * you don't provide explicit specialization for.
	 */
	template <class Type>
	struct DenseMapInfo
	{
		constexpr static Type GetEmptyKey() { return static_cast<Type>(-1); }
		constexpr static Type GetTombstoneKey() { return static_cast<Type>(-2); }
	};

	/** DenseMapInfo specialization that works for pointer types. */
	template <class Type>
	struct DenseMapInfo<Type*>
	{
		constexpr static Type* getEmptyKey() { return reinterpret_cast<Type*>(-1); }
		constexpr static Type* getTombstoneKey() { return reinterpret_cast<Type*>(-2); }
	};

	/** DenseMap iterator implementation. */
	template<class Key, class Value, class KeyInfo>
	class DenseMapIterator
	{
	public:
		typedef std::pair<Key, Value> DensePair;

		DenseMapIterator(const DensePair* ptr, const DensePair* end)
			:ptr(ptr), end(end)
		{
			next();
		}

		std::pair<Key, Value>& operator* () const { return *const_cast<DensePair*>(ptr); }
		std::pair<Key, Value>* operator-> () const { return const_cast<DensePair*>(ptr); }

		bool operator== (const DenseMapIterator& rhs) const { return ptr == rhs.ptr; }
		bool operator!= (const DenseMapIterator& rhs) const { return ptr != rhs.ptr; }

		DenseMapIterator& operator++ ()
		{
			++ptr;
			next();

			return *this;
		}

		DenseMapIterator operator++ (int)
		{
			DenseMapIterator tmp = *this;
			++*this;

			return tmp;
		}

	protected:
		const DensePair* ptr;
		const DensePair* end;

	private:
		void Next()
		{
			while (ptr != end && ((ptr->first == KeyInfo::getEmptyKey()) || (ptr->first == KeyInfo::getTombstoneKey())))
				++ptr;
		}
	};

	/** DenseMap constant iterator implementation. */
	template <class Key, class Value, class KeyInfo>
	class DenseMapConstIterator final : public DenseMapIterator<Key, Value, KeyInfo>
	{
	public:
		DenseMapConstIterator(const std::pair<Key, Value>* ptr, const std::pair<Key, Value>* end)
			:DenseMapIterator<Key, Value, KeyInfo>(ptr, end)
		{ }

		const std::pair<Key, Value>& operator* () const { return *this->ptr; }
		const std::pair<Key, Value>* operator-> () const { return this->ptr; }
	};

	/**
	 * Hash-map with densely stored values, using quadratic probing for lookup. 
	 * 
	 * @tparam Key		Type that represents the key in the map. Must have a specialization for DenseMapInfo<Type>, 
	 *					that provides a value representing an empty key value and a tombstone key value 
	 *					(@see DenseMapInfo<Type>). Also must have a @p bs_hash<T> specialization, and an equality
	 *					operator.
	 * @tparam	Value	Type of values to store in the hash-map.
	 * @tparam	KeyInfo	Type that provides information about empty and tombstone key values for the provided type. The
	 *					default value will handle pointers and unsigned integers, using -1 for empty key and -2 for
	 *					tombstone key value.
	 * @tparam	Size	Initial capacity of the map.
	 * 
	*/
	template<class Key, class Value, class KeyInfo = DenseMapInfo<Key>, UINT32 Size = 64>
	class DenseMap
	{
	public:
		typedef std::pair<Key, Value> DensePair;
		typedef DenseMapIterator<Key, Value, KeyInfo> Iterator;
		typedef DenseMapConstIterator<Key, Value, KeyInfo> ConstIterator;

		DenseMap() = default;
		DenseMap(const DenseMap<Key, Value>& other)
		{
			mCount = 0;
			if (!other.Empty())
				copy(other);
		}

		DenseMap(DenseMap<Key, Value>&& other)
		{
			mCount = 0;
			if (!other.Empty())
				copy(std::move(other));
		}

		DenseMap(UINT32 n)
		{
			append(n);
		}

		~DenseMap()
		{
			for (DensePair* pair = getBuckets(), *e = getBuckets() + getCount(); pair != e; ++pair)
			{
				if (!(pair->first == getEmptyKey()) && !(pair->first == getTombstoneKey()))
					pair->second.~Value();

				pair->first.~Key();
			}

			bs_free(mBuckets);
		}

		Value& operator[] (const Key& key)
		{
			return Construct(key).second;
		}

		DenseMap& operator= (const DenseMap<Key, Value>& other)
		{
			if (this == other)
				return *this;

			copy(other);
			return *this;
		}

		bool operator== (const DenseMap<Key, Value>& other) const
		{
			if (this->Size() != other.Size()) return false;
			return std::Equal(this->Begin(), this->end(), other.Begin());
		}

		bool operator!= (const DenseMap<Key, Value>& other) const
		{
			return !(*this == other);
		}

		Iterator Begin()
		{
			return Iterator(mBuckets, mBuckets + mCount);
		}

		Iterator End()
		{
			return Iterator(mBuckets + mCount, mBuckets + mCount);
		}

		ConstIterator Begin() const
		{
			return ConstIterator(mBuckets, mBuckets + mCount);
		}

		ConstIterator End() const
		{
			return ConstIterator(mBuckets + mCount, mBuckets + mCount);
		}

		bool Empty() const { return mEntries == 0; }
		UINT32 Size() const { return mEntries; }

		/** Grow so that it has at least n mBuckets. */
		void Resize(UINT32 n)
		{
			grow(n);
		}

		void Clear()
		{
			// If the capacity of the array is huge and the elements used is small then shrink the array.
			if (mEntries * 4 < mCount && mCount > Size)
			{
				shrink();
				return;
			}

			const Key emptyKey = getEmptyKey();
			for (DensePair* pair = getBuckets(), *e = getBuckets() + getCount(); pair != e; ++pair)
			{
				if (!(pair->first == emptyKey))
				{
					if (!(pair->first == getTombstoneKey()))
					{
						pair->second.~Value();
						--mEntries;
					}

					pair->first = emptyKey;
				}
			}

			assert(size() == 0 && "The number of nodes is imbalance");
			mTombstones = 0;
		}

		bool Contains(const Key& key) const
		{
			DensePair* temp;
			return Lookup(key, temp);
		}

		Iterator Find(const Key& key)
		{
			DensePair* temp;

			if (lookup(key, temp))
				return Iterator(temp, mBuckets + mCount);

			return End();
		}

		ConstIterator Find(const Key& key) const
		{
			DensePair* temp;

			if (lookup(key, temp))
				return ConstIterator(temp, mBuckets + mCount);

			return End();
		}

		std::pair<Iterator, bool> Insert(const std::pair<Key, Value>& pair)
		{
			if (empty())
				init();

			DensePair* temp;

			// It is already in map
			if (lookup(pair.first, temp))
				return std::make_pair(Iterator(temp, getBuckets() + getCount()), false);

			// Otherwise insert the new element
			temp = insertBucket(pair.first, pair.second, temp);

			return std::make_pair(Iterator(temp, getBuckets() + getCount()), true);
		}

		bool Erase(const Key& key)
		{
			assert(size() != 0);
			DensePair* temp;

			if (!lookup(key, temp))
				return false;

			update(temp);

			return true;
		}

		bool Erase(Iterator iter)
		{
			assert(size() != 0);
			DensePair* temp = &*iter;

			update(temp);

			return true;
		}

		DensePair& Construct(const Key& key)
		{
			DensePair* temp;

			if (lookup(key, temp))
				return temp;

			return InsertBucket(key, Value(), temp);
		}

	private:
		void Init()
		{
			append(Size);
		}

		void Update(DensePair* t)
		{
			t->second.~Value();
			t->first = getTombstoneKey();
			--mEntries;
			++mTombstones;
		}

		void Copy(const DenseMap<Key, Value>& other)
		{
			if (getCount() != 0 && (!std::is_pod<Key>() || !std::is_pod<Value>()))
			{
				const Key EmptyKey = getEmptyKey();
				const Key TombstoneKey = getTombstoneKey();
				for (DensePair* pair = getBuckets(), *e = getBuckets() + getCount(); pair != e; ++pair)
				{
					if (!(pair->first == EmptyKey) && !(pair->first == TombstoneKey))
						pair->second.~Value();

					pair->first.~Key();
				}
			}

			mEntries = other.Size();
			mTombstones = other.GetTombstones();

			if (getCount())
				bs_free(mBuckets);

			mBuckets = bs_allocN<DensePair>(other.GetCount());

			if (std::is_pod<Key>() && std::is_pod<Value>())
				memcpy(getBuckets(), other.GetBuckets(), other.getCount() * sizeof(DensePair));
			else
				for (UINT32 i = 0; i < other.GetCount(); ++i)
				{
					new (&mBuckets[i].first) Key(other.mBuckets[i].first);

					if (!(mBuckets[i].first == getEmptyKey()) && !(mBuckets[i].first == getTombstoneKey()))
						new (&mBuckets[i].second) Value(other.mBuckets[i].second);
				}

			mCount = other.GetCount();
		}

		DensePair* insertBucket(const Key& key, const Value& value, DensePair* temp)
		{
			// If the hash table load is more than 3/4 or is it fewer than 1/8 of the buckets then grow the table.
			// If there is one empty bucket with tons of tombstones, missing lookups would have to
			// probe almost the entire table until it found the empty bucket. If the
			// table is completely filled with tombstones, no lookup would ever succeed, making init loops in lookup.
			if (size() * 4 >= getCount() * 3 || getCount() - (size() + getTombstones()) < getCount() / 8)
			{
				this->Grow(getCount() * 2);
				lookup(key, temp);
			}

			++mEntries;

			// If we are writing over a tombstone, remember this
			if (!(temp->first == getEmptyKey()))
				--mTombstones;

			temp->first = key;
			new (&temp->second) Value(value);

			return temp;
		}

		/**
		 * Lookup the appropriate bucket for key, returning it in @p f. If the bucket contains the key and a value, 
		 * this returns true, otherwise it returns a bucket with an empty marker or tombstone and returns false.
		 */
		bool Lookup(const Key& key, DensePair*& f) const
		{
			UINT32 bucketN = getHashValue(key);
			UINT32 probe = 1;
			DensePair* bucket = getBuckets();

			// Keep track of whether we find a tombstone while probing.
			DensePair* foundTombstone = 0;

			const Key EmptyKey = getEmptyKey();
			const Key TombstoneKey = getTombstoneKey();
			assert(!(key == EmptyKey) && !(key == TombstoneKey) && "Empty/Tombstone value shouldn't be inserted into map!");

			while (1)
			{
				DensePair* temp = bucket + (bucketN & (getCount() - 1));

				// If found the key bucket return it.
				if ((temp->first == key))
				{
					f = temp;
					return true;
				}

				// If we found an empty bucket, the key doesn't exist in the set.
				// Insert it and return the default value.
				if ((temp->first == EmptyKey))
				{
					// If we've already seen a tombstone while probing, fill it in instead
					// of the empty bucket we eventually probed to.
					if (foundTombstone)
						temp = foundTombstone;

					f = foundTombstone ? foundTombstone : temp;

					return false;

				}

				// If this is a tombstone, remember it.  If key ends up not in the map, we
				// prefer to return it than something that would require more probing.
				if ((temp->first == TombstoneKey) && !foundTombstone)
					foundTombstone = temp;

				// Otherwise it's a hash collision or a tombstone, continue quadratic probing.
				bucketN += probe++;
			}
		}

		void Append(UINT32 n)
		{
			mEntries = 0;
			mTombstones = 0;
			mCount = n;
			assert(n && (n & (n - 1)) == 0 && "initial buckets must be a power of two!");

			mBuckets = bs_allocN<DensePair>(n);

			for (UINT32 i = 0; i != n; ++i)
				new (&mBuckets[i].first) Key(getEmptyKey());
		}

		void Grow(UINT32 n)
		{
			const UINT32 oldCount = getCount();
			DensePair* oldBuckets = getBuckets();

			// Double the number of buckets
			while (mCount <= n)
				mCount <<= 1;

			mTombstones = 0;
			mBuckets = bs_allocN<DensePair>(getCount());

			// Initialize all the keys to EmptyKey
			const Key EmptyKey = getEmptyKey();
			for (UINT32 i = 0, e = mCount; i != e; ++i)
				new (&mBuckets[i].first) Key(EmptyKey);

			// Insert all the old elements
			const Key TombstoneKey = getTombstoneKey();
			for (DensePair* pair = oldBuckets, *e = oldBuckets + oldCount; pair != e; ++pair)
			{
				if (!(pair->first == EmptyKey) && !(pair->first == TombstoneKey))
				{
					// Insert the pair into the new table
					DensePair* destBucket;
					bool f = lookup(pair->first, destBucket);

					// silence warning
					f = f;
					assert(!f && "Key already in new map");

					destBucket->first = pair->first;
					new (&destBucket->second) Value(pair->second);

					// Free the value
					pair->second.~Value();
				}

				pair->first.~Key();
			}
			// Free the old table
			bs_free(oldBuckets);
		}

		void Shrink()
		{
			const UINT32 oldCount = getCount();
			DensePair* oldBuckets = getBuckets();

			// Reduce the number of mBuckets
			mCount = size() > 32 ? 1 << ((int)std::log(size()) + 1) : 64;
			mTombstones = 0;
			mBuckets = bs_allocN<DensePair>(getCount());

			// Initialize the keys to EmptyKey
			const Key EmptyKey = getEmptyKey();
			for (UINT32 i = 0, e = mCount; i != e; ++i)
				new (&mBuckets[i].first) Key(EmptyKey);

			// Free the old mBuckets
			const Key TombstoneKey = getTombstoneKey();
			for (DensePair* B = oldBuckets, *E = oldBuckets + oldCount; B != E; ++B)
			{
				if (!(B->first == EmptyKey) && !(B->first == TombstoneKey))
					B->second.~Value();

				B->first.~Key();
			}

			// Free the old mBuckets
			bs_free(oldBuckets);
			mEntries = 0;
		}

		UINT32 GetCount() const { return mCount; }
		DensePair* getBuckets() const { return mBuckets; }
		UINT32 GetTombstones() const { return mTombstones; }

		static UINT32 GetHashValue(const Key& key) { return bs_hash(key); }
		static const Key GetEmptyKey() { return KeyInfo::getEmptyKey(); }
		static const Key GetTombstoneKey() { return KeyInfo::getTombstoneKey(); }

	private:
		DensePair* mBuckets = nullptr;
		UINT32 mCount = 0;
		UINT32 mEntries = 0;
		UINT32 mTombstones = 0;
	};

	/** @} */
}
