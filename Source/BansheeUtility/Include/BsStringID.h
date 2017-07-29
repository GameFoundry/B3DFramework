//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsSpinLock.h"

namespace bs
{
	/** @addtogroup String
	 *  @{
	 */

	/**
	 * A string identifier that provides very fast comparisons to other string identifiers. Significantly faster than
	 * comparing raw strings.
	 *
	 * @note	
	 * Essentially a unique ID is generated for each string and then the ID is used for comparisons as if you were using 
	 * an integer or an enum.
	 * @note
	 * Thread safe.
	 */
	class BS_UTILITY_EXPORT StringID
	{
		static const int HASH_TABLE_SIZE = 4096;
		static const int MAX_CHUNK_COUNT = 50;
		static const int ELEMENTS_PER_CHUNK = 256;
		static const int STRING_SIZE = 256;

		/** Helper class that performs string actions on both null terminated character arrays and standard strings. */
		template<class T>
		class StringIDUtil
		{
		public:
			static UINT32 size(T const& input) { return 0; }
			static void copy(T const& input, char* dest) { }
			static bool compare(T const& a, char* b) { return 0; }
		};

		/**	Internal data that is shared by all instances for a specific string. */
		struct InternalData
		{
			UINT32 id;
			InternalData* next;
			char chars[STRING_SIZE];
		};

		/**	Performs initialization of static members as soon as the library is loaded. */
		struct InitStatics
		{
			InitStatics();
		};

	public:
		StringID();

		StringID(const char* name)
			:mData(nullptr)
		{
			construct(name);
		}

		StringID(const String& name)
			:mData(nullptr)
		{
			construct(name);
		}

		template<int N>
		StringID(const char name[N])
			:mData(nullptr)
		{
			construct((const char*)name);
		}

		/**	Compare to string ids for equality. Uses fast integer comparison. */
		bool operator== (const StringID& rhs) const
		{
			return mData == rhs.mData;
		}

		/**	Compare to string ids for inequality. Uses fast integer comparison. */
		bool operator!= (const StringID& rhs) const
		{
			return mData != rhs.mData;
		}

		/**	Returns true if the string id has no value assigned. */
		bool empty() const
		{
			return mData == nullptr;
		}

		/**	Returns the null-terminated name of the string id. */
		const char* cstr() const
		{
			if (mData == nullptr)
				return nullptr;

			return mData->chars;
		}

		/** Returns the unique identifier of the string. */
		UINT32 id() const { return mData ? mData->id : -1; }

		static const StringID NONE;

	private:
		/**Constructs a StringID object in a way that works for pointers to character arrays and standard strings. */
		template<class T>
		void construct(T const& name);

		/**	Calculates a hash value for the provided null-terminated string. */
		template<class T>
		UINT32 calcHash(T const& input);

		/**
		 * Allocates a new string entry and assigns it a unique ID. Optionally expands the chunks buffer if the new entry 
		 * doesn't fit.
		 */
		InternalData* allocEntry();

		InternalData* mData;

		static volatile InitStatics mInitStatics;
		static InternalData* mStringHashTable[HASH_TABLE_SIZE];
		static InternalData* mChunks[MAX_CHUNK_COUNT];

		static UINT32 mNextId;
		static UINT32 mNumChunks;
		static SpinLock mSync;
	};

	/** @cond SPECIALIZATIONS */

	template<> struct RTTIPlainType <StringID>
	{
		enum { id = TID_StringID }; enum { hasDynamicSize = 1 };

		static void toMemory(const StringID& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			bool isEmpty = data.empty();
			memory = rttiWriteElem(isEmpty, memory);

			if (!isEmpty)
			{
				UINT32 length = (UINT32)strlen(data.cstr());
				memcpy(memory, data.cstr(), length * sizeof(char));
			}
		}

		static UINT32 fromMemory(StringID& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			bool empty = false;
			memory = rttiReadElem(empty, memory);

			if (!empty)
			{
				UINT32 length = (size - sizeof(UINT32) - sizeof(bool)) / sizeof(char);

				char* name = (char*)bs_alloc(length + 1);
				memcpy(name, memory, length);
				name[length] = '\0';

				data = StringID(name);
			}

			return size;
		}

		static UINT32 getDynamicSize(const StringID& data)
		{
			UINT32 dataSize = sizeof(bool) + sizeof(UINT32);

			bool isEmpty = data.empty();
			if (!isEmpty)
			{
				UINT32 length = (UINT32)strlen(data.cstr());
				dataSize += length * sizeof(char);
			}

			return (UINT32)dataSize;
		}
	};

	/** @endcond */
	/** @} */
}

/** @cond STDLIB */
/** @addtogroup String
 *  @{
 */

namespace std
{
/**	Hash value generator for StringID. */
template<>
struct hash<bs::StringID>
{
	size_t operator()(const bs::StringID& value) const
	{
		return (size_t)value.id();
	}
};
}

/** @} */
/** @endcond */