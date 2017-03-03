//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		T get(UINT32 idx);

		/** Sets an entry from the array at the specified index. */
		template<class T>
		void set(UINT32 idx, const T& value);

		/** Returns the raw object from the array at the specified index. */
		template<class T>
		T* getRawPtr(UINT32 idx = 0)
		{
#if BS_DEBUG_MODE
			assert(sizeof(T) == elementSize());
#endif

			return (T*)getArrayAddr(mInternal, sizeof(T), idx);
		}

		/** 
		 * Returns the raw object from the array at the specified index. Provided size determines the size of each
		 * element in the array. Caller must ensure it is correct for the specified array.
		 */
		UINT8* getRawPtr(UINT32 size, UINT32 idx)
		{
#if BS_DEBUG_MODE
			assert(size == elementSize());
#endif

			return getArrayAddr(mInternal, size, idx);
		}

		/** 
		 * Creates a new array of managed objects. 
		 *
		 * @tparam	T	ScriptObject wrapper for the specified managed type.
		 */
		template<class T>
		static ScriptArray create(UINT32 size);

		/** Returns number of elements in the array. */
		UINT32 size() const;

		/** Returns the size of an individual element in the array, in bytes. */
		UINT32 elementSize() const;

		/** Returns the managed object representing this array. */
		MonoArray* getInternal() const { return mInternal; }

		/** 
		 * Returns the address of an array item at the specified index. 
		 *
		 * @param[in]	array	Array from which to retrieve the item.
		 * @param[in]	size	Size of a single item in the array.
		 * @param[in]	idx		Index of the item to retrieve.
		 * @return				Address of the array item at the requested index. 
		 */
		static UINT8* getArrayAddr(MonoArray* array, UINT32 size, UINT32 idx);

		/** Returns the class of the elements within an array class. */
		static ::MonoClass* getElementClass(::MonoClass* arrayClass);

		/** Returns the rank of the provided array class. */
		static UINT32 getRank(::MonoClass* arrayClass);

		/** Builds an array class from the provided element class and a rank. */
		static ::MonoClass* buildArrayClass(::MonoClass* elementClass, UINT32 rank);
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
			return *(T*)ScriptArray::getArrayAddr(array, sizeof(T), idx);
		}

		template<class T>
		void ScriptArray_set(MonoArray* array, UINT32 idx, const T& value)
		{
			T* item = (T*)ScriptArray::getArrayAddr(array, sizeof(T), idx);
			*item = value;
		}

		template<>
		BS_MONO_EXPORT String ScriptArray_get(MonoArray* array, UINT32 idx);

		template<>
		BS_MONO_EXPORT WString ScriptArray_get(MonoArray* array, UINT32 idx);

		template<>
		BS_MONO_EXPORT void ScriptArray_set<String>(MonoArray* array, UINT32 idx, const String& value);

		template<>
		BS_MONO_EXPORT void ScriptArray_set<WString>(MonoArray* array, UINT32 idx, const WString& value);

		template<class T>
		inline ScriptArray ScriptArray_create(UINT32 size)
		{
			return ScriptArray(*T::getMetaData()->scriptClass, size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT8>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getByteClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT8>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getSByteClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT16>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getUINT16Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT16>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getINT16Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT32>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getUINT32Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT32>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getINT32Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<UINT64>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getUINT64Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<INT64>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getINT64Class(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<WString>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getStringClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<String>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getStringClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<float>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getFloatClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<double>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getDoubleClass(), size);
		}

		template<>
		inline ScriptArray ScriptArray_create<bool>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getBoolClass(), size);
		}
	}

	/** @} */

	template<class T>
	T ScriptArray::get(UINT32 idx)
	{
		return Detail::ScriptArray_get<T>(mInternal, idx);
	}

	/** Sets an entry from the array at the specified index. */
	template<class T>
	void ScriptArray::set(UINT32 idx, const T& value)
	{
		Detail::ScriptArray_set<T>(mInternal, idx, value);
	}

	template<class T>
	ScriptArray ScriptArray::create(UINT32 size)
	{
		return Detail::ScriptArray_create<T>(size);
	}
}