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

		/** Assigns a value to the specified index. */
		template<class T>
		void set(UINT32 idx, const T& value);

		/** 
		 * Assigns some data represented as raw memory to the array at the specified index. User must provide the size of
		 * the data, and it must match the element size expected by the array. Multiple array elements can be provided
		 * sequentially by setting the @p count parameter.
		 */
		void setRaw(UINT32 idx, const UINT8* value, UINT32 size, UINT32 count = 1);

		/** 
		 * Returns the raw memory of the data at the specified array index. Returned value should not be used for writing
		 * to the array and set() or setRaw() methods should be used instead.
		 */
		UINT8* getRaw(UINT32 idx, UINT32 size)
		{
#if BS_DEBUG_MODE
			assert(size == elementSize());
#endif
			return _getArrayAddr(mInternal, size, idx);
		}
		/** 
		 * Returns the raw memory of the data at the specified array index. Returned value should not be used for writing
		 * to the array and set() or setRaw() methods should be used instead.
		 */
		template<class T>
		T* getRaw(UINT32 idx = 0)
		{
#if BS_DEBUG_MODE
			assert(sizeof(T) == elementSize());
#endif
			return (T*)_getArrayAddr(mInternal, sizeof(T), idx);
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

		/** Returns the class of the elements within an array class. */
		static ::MonoClass* getElementClass(::MonoClass* arrayClass);

		/** Returns the rank of the provided array class. */
		static UINT32 getRank(::MonoClass* arrayClass);

		/** Builds an array class from the provided element class and a rank. */
		static ::MonoClass* buildArrayClass(::MonoClass* elementClass, UINT32 rank);

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
		static UINT8* _getArrayAddr(MonoArray* array, UINT32 size, UINT32 idx);

		/** 
		 * Sets one or multiple entries from the array at the specified index, from raw memory. User must provide the size
		 * of the element, and it must match the element size expected by the array.
		 */
		static void _setArrayVal(MonoArray* array, UINT32 idx, const UINT8* value, UINT32 size, UINT32 count = 1);

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
			return *(T*)ScriptArray::_getArrayAddr(array, sizeof(T), idx);
		}

		template<class T>
		void ScriptArray_set(MonoArray* array, UINT32 idx, const T& value)
		{
			ScriptArray::_setArrayVal(array, idx, (UINT8*)&value, sizeof(T));
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

		template<>
		inline ScriptArray ScriptArray_create<MonoObject*>(UINT32 size)
		{
			return ScriptArray(MonoUtil::getObjectClass(), size);
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