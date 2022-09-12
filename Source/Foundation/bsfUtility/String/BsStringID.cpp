//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "String/BsStringID.h"

namespace bs
{
	const StringID StringID::NONE;

	volatile StringID::InitStatics StringID::mInitStatics = StringID::InitStatics();

	StringID::InternalData* StringID::mStringHashTable[HASH_TABLE_SIZE];
	StringID::InternalData* StringID::mChunks[MAX_CHUNK_COUNT];

	UINT32 StringID::mNextId = 0;
	UINT32 StringID::mNumChunks = 0;
	SpinLock StringID::mSync;

	StringID::InitStatics::InitStatics()
	{
		ScopedSpinLock Lock(mSync);

		memset(mStringHashTable, 0, sizeof(mStringHashTable));
		memset(mChunks, 0, sizeof(mChunks));

		mChunks[0] = (InternalData*)bs_alloc(sizeof(InternalData) * ELEMENTS_PER_CHUNK);
		memset(mChunks[0], 0, sizeof(InternalData) * ELEMENTS_PER_CHUNK);

		mNumChunks++;
	}

	template<class T>
	void StringID::Construct(T const& name)
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

		ScopedSpinLock Lock(mSync);

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
	UINT32 StringID::CalcHash(T const& input)
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
		static UINT32 Size(const char* const& input) { return (UINT32)strlen(input); }
		static void Copy(const char* const& input, char* dest) { memcpy(dest, input, strlen(input) + 1); }
		static bool Compare(const char* const& a, char* b) { return strcmp(a, b) == 0; }
	};

	template<>
	class StringID::StringIDUtil <String>
	{
	public:
		static UINT32 Size(String const& input) { return (UINT32)input.length(); }
		static void Copy(String const& input, char* dest)
		{
			UINT32 len = (UINT32)input.length();
			input.copy(dest, len);
			dest[len] = '\0';
		}
		static bool Compare(String const& a, char* b) { return a.compare(b) == 0; }
	};

	template BS_UTILITY_EXPORT void StringID::Construct(const char* const&);
	template BS_UTILITY_EXPORT void StringID::Construct(String const&);
	
	template BS_UTILITY_EXPORT UINT32 StringID::CalcHash(const char* const&);
	template BS_UTILITY_EXPORT UINT32 StringID::CalcHash(String const&);
}
