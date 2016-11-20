//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsStringID.h"

namespace bs
{
	const StringID StringID::NONE = StringID();

	volatile StringID::InitStatics StringID::mInitStatics = StringID::InitStatics();

	StringID::InternalData* StringID::mStringHashTable[HASH_TABLE_SIZE];
	StringID::InternalData* StringID::mChunks[MAX_CHUNK_COUNT];

	UINT32 StringID::mNextId = 0;
	UINT32 StringID::mNumChunks = 0;
	SpinLock StringID::mSync;

	StringID::InitStatics::InitStatics()
	{
		ScopedSpinLock lock(mSync);

		memset(mStringHashTable, 0, sizeof(mStringHashTable));
		memset(mChunks, 0, sizeof(mChunks));

		mChunks[0] = (InternalData*)bs_alloc(sizeof(InternalData) * ELEMENTS_PER_CHUNK);
		memset(mChunks[0], 0, sizeof(InternalData) * ELEMENTS_PER_CHUNK);

		mNumChunks++;
	}

	StringID::StringID()
		:mData(nullptr)
	{ }

	template<class T>
	void StringID::construct(T const& name)
	{
		assert(StringIDUtil<T>::size(name) <= STRING_SIZE);

		UINT32 hash = calcHash(name) & (sizeof(mStringHashTable) / sizeof(mStringHashTable[0]) - 1);
		InternalData* existingEntry = mStringHashTable[hash];
		
		while (existingEntry != nullptr)
		{
			if (StringIDUtil<T>::compare(name, existingEntry->chars))
			{
				mData = existingEntry;
				return;
			}

			existingEntry = existingEntry->next;
		}

		ScopedSpinLock lock(mSync);

		// Search for the value again in case other thread just added it
		existingEntry = mStringHashTable[hash];
		InternalData* lastEntry = nullptr;
		while (existingEntry != nullptr)
		{
			if (StringIDUtil<T>::compare(name, existingEntry->chars))
			{
				mData = existingEntry;
				return;
			}

			lastEntry = existingEntry;
			existingEntry = existingEntry->next;
		}

		mData = allocEntry();
		StringIDUtil<T>::copy(name, mData->chars);

		if (lastEntry == nullptr)
			mStringHashTable[hash] = mData;
		else
			lastEntry->next = mData;
	}

	template<class T>
	UINT32 StringID::calcHash(T const& input)
	{
		UINT32 size = StringIDUtil<T>::size(input);

		UINT32 hash = 0;
		for (UINT32 i = 0; i < size; i++)
			hash = hash * 101 + input[i];

		return hash;
	}

	StringID::InternalData* StringID::allocEntry()
	{
		UINT32 chunkIdx = mNextId / ELEMENTS_PER_CHUNK;

		assert(chunkIdx < MAX_CHUNK_COUNT);
		assert(chunkIdx <= mNumChunks); // Can only increment sequentially

		if (chunkIdx >= mNumChunks)
		{
			mChunks[chunkIdx] = (InternalData*)bs_alloc(sizeof(InternalData) * ELEMENTS_PER_CHUNK);
			memset(mChunks[chunkIdx], 0, sizeof(InternalData) * ELEMENTS_PER_CHUNK);

			mNumChunks++;
		}

		InternalData* chunk = mChunks[chunkIdx];
		UINT32 chunkSpecificIndex = mNextId % ELEMENTS_PER_CHUNK;

		InternalData* newEntry = &chunk[chunkSpecificIndex];
		newEntry->id = mNextId++;
		newEntry->next = nullptr;

		return newEntry;
	}

	template<>
	class StringID::StringIDUtil<const char*>
	{
	public:
		static UINT32 size(const char* const& input) { return (UINT32)strlen(input); }
		static void copy(const char* const& input, char* dest) { memcpy(dest, input, strlen(input) + 1); }
		static bool compare(const char* const& a, char* b) { return strcmp(a, b) == 0; }
	};

	template<>
	class StringID::StringIDUtil <String>
	{
	public:
		static UINT32 size(String const& input) { return (UINT32)input.length(); }
		static void copy(String const& input, char* dest)
		{ 
			UINT32 len = (UINT32)input.length();
			input.copy(dest, len);
			dest[len] = '\0';
		}
		static bool compare(String const& a, char* b) { return a.compare(b) == 0; }
	};

	template class StringID::StringIDUtil <const char*>;
	template class StringID::StringIDUtil <String>;

	template BS_UTILITY_EXPORT void StringID::construct(const char* const&);
	template BS_UTILITY_EXPORT void StringID::construct(String const&);
	
	template BS_UTILITY_EXPORT UINT32 StringID::calcHash(const char* const&);
	template BS_UTILITY_EXPORT UINT32 StringID::calcHash(String const&);
}