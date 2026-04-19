//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Threading/B3DSpinLock.h"

namespace b3d
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
	class B3D_EXPORT StringID
	{
		static constexpr const int kHashTableSize = 4096;
		static constexpr const int kMaxChunkCount = 50;
		static constexpr const int kElementsPerChunk = 256;
		static constexpr const int kStringSize = 256;

		/** Helper class that performs string actions on both null terminated character arrays and standard strings. */
		template <class T>
		class StringIDUtil
		{
		public:
			static u32 Size(T const& input) { return 0; }

			static void Copy(T const& input, char* dest) {}

			static bool Compare(T const& a, char* b) { return 0; }
		};

		/**	Internal data that is shared by all instances for a specific string. */
		struct InternalData
		{
			u32 Id;
			InternalData* Next;
			char Chars[kStringSize];
		};

		/**	Performs initialization of static members as soon as the library is loaded. */
		struct InitStatics
		{
			InitStatics();
		};

	public:
		StringID() = default;

		StringID(const char* name)
		{
			Construct(name);
		}

		StringID(const String& name)
		{
			Construct(name);
		}

		template <int N>
		StringID(const char name[N])
		{
			Construct((const char*)name);
		}

		/**	Compare to string ids for equality. Uses fast integer comparison. */
		bool operator==(const StringID& rhs) const
		{
			return mData == rhs.mData;
		}

		/**	Compare to string ids for inequality. Uses fast integer comparison. */
		bool operator!=(const StringID& rhs) const
		{
			return mData != rhs.mData;
		}

		/** Implicitly converts to a normal string. */
		operator String() const { return String(mData->Chars); }

		/** Implicitly converts to a string view. */
		operator StringView() const { return mData->Chars; }

		/**	Returns true if the string id has no value assigned. */
		bool Empty() const
		{
			return mData == nullptr;
		}

		/**	Returns the null-terminated name of the string id. */
		const char* CStr() const
		{
			if(mData == nullptr)
				return "";

			return mData->Chars;
		}

		/** Returns the unique identifier of the string. */
		u32 Id() const { return mData ? mData->Id : -1; }

		static const StringID kNone;

	private:
		/**Constructs a StringID object in a way that works for pointers to character arrays and standard strings. */
		template <class T>
		void Construct(T const& name);

		/**	Calculates a hash value for the provided null-terminated string. */
		template <class T>
		u32 CalcHash(T const& input);

		/**
		 * Allocates a new string entry and assigns it a unique ID. Optionally expands the chunks buffer if the new entry
		 * doesn't fit.
		 */
		InternalData* AllocEntry();

		InternalData* mData = nullptr;

		static volatile InitStatics mInitStatics;
		static InternalData* mStringHashTable[kHashTableSize];
		static InternalData* mChunks[kMaxChunkCount];

		static u32 mNextId;
		static u32 mNumChunks;
		static SpinLock mSync;
	};

	/** @} */
} // namespace b3d

/** @cond STDLIB */
/** @addtogroup String
 *  @{
 */

namespace std
{
	/**	Hash value generator for StringID. */
	template <>
	struct hash<b3d::StringID>
	{
		size_t operator()(const b3d::StringID& value) const
		{
			return (size_t)value.Id();
		}
	};
} // namespace std

/** @} */
/** @endcond */
