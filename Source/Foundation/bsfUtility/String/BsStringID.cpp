//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "String/BsStringID.h"

using namespace bs;

const StringID StringID::kNone;

volatile StringID::InitStatics StringID::mInitStatics = StringID::InitStatics();

StringID::InternalData* StringID::mStringHashTable[kHashTableSize];
StringID::InternalData* StringID::mChunks[kMaxChunkCount];

u32 StringID::mNextId = 0;
u32 StringID::mNumChunks = 0;
SpinLock StringID::mSync;

StringID::InitStatics::InitStatics()
{
	ScopedSpinLock lock(mSync);

	memset(mStringHashTable, 0, sizeof(mStringHashTable));
	memset(mChunks, 0, sizeof(mChunks));

	mChunks[0] = (InternalData*)bs_alloc(sizeof(InternalData) * kElementsPerChunk);
	memset(mChunks[0], 0, sizeof(InternalData) * kElementsPerChunk);

	mNumChunks++;
}

template <class T>
void StringID::Construct(T const& name)
{
	assert(StringIDUtil<T>::Size(name) <= kStringSize);

	u32 hash = CalcHash(name) & (sizeof(mStringHashTable) / sizeof(mStringHashTable[0]) - 1);
	InternalData* existingEntry = mStringHashTable[hash];

	while(existingEntry != nullptr)
	{
		if(StringIDUtil<T>::Compare(name, existingEntry->Chars))
		{
			mData = existingEntry;
			return;
		}

		existingEntry = existingEntry->Next;
	}

	ScopedSpinLock lock(mSync);

	// Search for the value again in case other thread just added it
	existingEntry = mStringHashTable[hash];
	InternalData* lastEntry = nullptr;
	while(existingEntry != nullptr)
	{
		if(StringIDUtil<T>::Compare(name, existingEntry->Chars))
		{
			mData = existingEntry;
			return;
		}

		lastEntry = existingEntry;
		existingEntry = existingEntry->Next;
	}

	mData = AllocEntry();
	StringIDUtil<T>::Copy(name, mData->Chars);

	if(lastEntry == nullptr)
		mStringHashTable[hash] = mData;
	else
		lastEntry->Next = mData;
}

template <class T>
u32 StringID::CalcHash(T const& input)
{
	u32 size = StringIDUtil<T>::Size(input);

	u32 hash = 0;
	for(u32 i = 0; i < size; i++)
		hash = hash * 101 + input[i];

	return hash;
}

StringID::InternalData* StringID::AllocEntry()
{
	u32 chunkIdx = mNextId / kElementsPerChunk;

	assert(chunkIdx < kMaxChunkCount);
	assert(chunkIdx <= mNumChunks); // Can only increment sequentially

	if(chunkIdx >= mNumChunks)
	{
		mChunks[chunkIdx] = (InternalData*)bs_alloc(sizeof(InternalData) * kElementsPerChunk);
		memset(mChunks[chunkIdx], 0, sizeof(InternalData) * kElementsPerChunk);

		mNumChunks++;
	}

	InternalData* chunk = mChunks[chunkIdx];
	u32 chunkSpecificIndex = mNextId % kElementsPerChunk;

	InternalData* newEntry = &chunk[chunkSpecificIndex];
	newEntry->Id = mNextId++;
	newEntry->Next = nullptr;

	return newEntry;
}

template <>
class StringID::StringIDUtil<const char*>
{
public:
	static u32 Size(const char* const& input) { return (u32)strlen(input); }

	static void Copy(const char* const& input, char* dest) { memcpy(dest, input, strlen(input) + 1); }

	static bool Compare(const char* const& a, char* b) { return strcmp(a, b) == 0; }
};

template <>
class StringID::StringIDUtil<String>
{
public:
	static u32 Size(String const& input) { return (u32)input.length(); }

	static void Copy(String const& input, char* dest)
	{
		u32 len = (u32)input.length();
		input.copy(dest, len);
		dest[len] = '\0';
	}

	static bool Compare(String const& a, char* b) { return a.compare(b) == 0; }
};

template BS_UTILITY_EXPORT void StringID::Construct(const char* const&);
template BS_UTILITY_EXPORT void StringID::Construct(String const&);

template BS_UTILITY_EXPORT u32 StringID::CalcHash(const char* const&);
template BS_UTILITY_EXPORT u32 StringID::CalcHash(String const&);
