//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsMonoPrerequisites.h"
#include "BsScriptMeta.h"
#include "BsMonoUtil.h"
#include "BsMonoManager.h"

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	/** Helper class for creating and parsing managed arrays.*/
	class BS_MONO_EXPORT ScriptArray
	{
	public:
		/** Wraps an existing array and allows you to get/set its values. */
		ScriptArray(MonoArray* existingArray);
		/** Creates a new array of specified size with elements of the specified type. */
		ScriptArray(MonoClass& klass, UINT32 size);
		/** Creates a new array of specified size with elements of the specified type. */
		ScriptArray(::MonoClass* klass, UINT32 size);

		/** Retrieves an entry from the array at the specified index. */
		template<class T>
		T Get(UINT32 idx);

		/** Assigns a value to the specified index. */
		template<class T>
		void Set(UINT32 idx, const T& value);

		/**
		 * Assigns some data represented as raw memory to the array at the specified index. User must provide the size of
		 * the data, and it must match the element size expected by the array. Multiple array elements can be provided
		 * sequentially by setting the @p count parameter.
		 */
		void SetRaw(UINT32 idx, const UINT8* value, UINT32 size, UINT32 count = 1);

		/**
		 * Returns the raw memory of the data at the specified array index. Returned value should not be used for writing
		 * to the array and set() or setRaw() methods should be used instead.
		 */
		UINT8* GetRaw(UINT32 idx, UINT32 size)
		{
#if BS_DEBUG_MODE
			assert(size == ElementSize());
#endif
			return GetArrayAddrInternal(mInternal, size, idx);
		}
		/**
		 * Returns the raw memory of the data at the specified array index. Returned value should not be used for writing
		 * to the array and set() or setRaw() methods should be used instead.
		 */
		template<class T>
		T* GetRaw(UINT32 idx = 0)
		{
#if BS_DEBUG_MODE
			assert(sizeof(T) == elementSize());
#endif
			return (T*)GetArrayAddrInternal(mInternal, sizeof(T), idx);
		}

		/**
		 * Creates a new array of managed objects.
		 *
		 * @tparam	T	ScriptObject wrapper for the specified managed type.
		 */
		template<class T>
		static ScriptArray Create(UINT32 size);

		/** Returns number of elements in the array. */
		UINT32 Size() const;

		/** Returns the size of an individual element in the array, in bytes. */
		UINT32 ElementSize() const;

		/** Returns the managed object representing this array. */
		MonoArray* GetInternal() const { return mInternal; }

		/** Returns the class of the elements within an array class. */
		static ::MonoClass* GetElementClass(::MonoClass* arrayClass);

		/** Returns the rank of the provided array class. */
		static UINT32 GetRank(::MonoClass* arrayClass);

		/** Builds an array class from the provided element class and a rank. */
		static ::MonoClass* BuildArrayClass(::MonoClass* elementClass, UINT32 rank);

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Returns the address of an array item at the specified index.
		 *
		 * @param[in]	array	Array from which to retrieve the item.
		 * @param[in]	size	Size of a single item in the array.
		 * @param[in]	idx		Index of the item to retrieve.
		 * @return				Address of the array item at the requested index.
		 */
		static UINT8* GetArrayAddrInternal(MonoArray* array, UINT32 size, UINT32 idx);

		/**
		 * Sets one or multiple entries from the array at the specified index, from raw memory. User must provide the size
		 * of the element, and it must match the element size expected by the array.
		 */
		static void SetArrayValInternal(MonoArray* array, UINT32 idx, const UINT8* value, UINT32 size, UINT32 count = 1);

		/**
		 * @}
		 */
	private:

		MonoArray* mInternal;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */
	namespace Detail
	{
		// A layer of indirection for all methods specialized by ScriptArray. */

		template<class T>
		T ScriptArray_get(MonoArray* array, UINT32 idx)
		{
			return *(T*)ScriptArray::GetArrayAddrInternal(array, sizeof(T), idx);
		}

		template<class T>
		void ScriptArray_set(MonoArray* array, UINT32 idx, const T& value)
		{
			ScriptArray::SetArrayValInternal(array, idx, (UINT8*)&value, sizeof(T));
		}

		template<>
		BS_MONO_EXPORT String ScriptArray_get(MonoArray* array, UINT32 idx);

		template<>
		BS_MONO_EXPORT WString ScriptArray_get(MonoArray* array, UINT32 idx);

		template<>
		BS_MONO_EXPORT Path ScriptArray_get(MonoArray* array, UINT32 idx);

		template<>
		BS_MONO_EXPORT void ScriptArray_set<String>(MonoArray* array, UINT32 idx, const String& value);

		template<>
		BS_MONO_EXPORT void ScriptArray_set<WString>(MonoArray* array, UINT32 idx, const WString& value);

		template<>
		BS_MONO_EXPORT void ScriptArray_set<Path>(MonoArray* array, UINT32 idx, const Path& value);

		template<>
		BS_MONO_EXPORT void ScriptArray_set<std::nullptr_t>(MonoArray* array, UINT32 idx, const std::nullptr_t& value);

		template<class T>
		inline ScriptArray ScriptArray_create(UINT32 size)
		{
			return ScriptArray(*T::GetMetaData()->scriptClass, size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT8>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetByteClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT8>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetSByteClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<char>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetSByteClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT16>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetUinT16Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT16>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetInT16Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT32>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetUinT32Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT32>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetInT32Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT64>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetUinT64Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT64>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetInT64Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<WString>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetStringClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<String>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetStringClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<Path>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetStringClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<float>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetFloatClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<double>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetDoubleClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<bool>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetBoolClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<MonoObject*>(UINT32 size)
		{
			return ScriptArray(MonoUtil::GetObjectClass(), size);
		}
	}

	/** @} */

	template<class T>
	T ScriptArray::Get(UINT32 idx)
	{
		return Detail::ScriptArray_get<T>(mInternal, idx);
	}

	/** Sets an entry from the array at the specified index. */
	template<class T>
	void ScriptArray::Set(UINT32 idx, const T& value)
	{
		Detail::ScriptArray_set<T>(mInternal, idx, value);
	}

	template<class T>
	ScriptArray ScriptArray::Create(UINT32 size)
	{
		return Detail::ScriptArray_create<T>(size);
	}
}
