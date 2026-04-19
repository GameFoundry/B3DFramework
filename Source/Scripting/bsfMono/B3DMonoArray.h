//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"
#include "B3DScriptTypeMetaData.h"
#include "B3DMonoUtil.h"
#include "B3DMonoManager.h"

namespace b3d
{
	/** @addtogroup Mono
	 *  @{
	 */

	/** Helper class for creating and parsing managed arrays.*/
	class B3D_MONO_EXPORT ScriptArray
	{
	public:
		/** Wraps an existing array and allows you to get/set its values. */
		ScriptArray(MonoArray* existingArray);
		/** Creates a new array of specified size with elements of the specified type. */
		ScriptArray(MonoClass& klass, u32 size);
		/** Creates a new array of specified size with elements of the specified type. */
		ScriptArray(::MonoClass* klass, u32 size);

		/** Retrieves an entry from the array at the specified index. */
		template <class T>
		T Get(u32 idx);

		/** Assigns a value to the specified index. */
		template <class T>
		void Set(u32 idx, const T& value);

		/**
		 * Assigns some data represented as raw memory to the array at the specified index. User must provide the size of
		 * the data, and it must match the element size expected by the array. Multiple array elements can be provided
		 * sequentially by setting the @p count parameter.
		 */
		void SetRaw(u32 idx, const u8* value, u32 size, u32 count = 1);

		/**
		 * Returns the raw memory of the data at the specified array index. Returned value should not be used for writing
		 * to the array and set() or setRaw() methods should be used instead.
		 */
		u8* GetRaw(u32 idx, u32 size)
		{
#if B3D_DEBUG
			B3D_ASSERT(size == ElementSize());
#endif
			return GetArrayAddrInternal(mInternal, size, idx);
		}

		/**
		 * Returns the raw memory of the data at the specified array index. Returned value should not be used for writing
		 * to the array and set() or setRaw() methods should be used instead.
		 */
		template <class T>
		T* GetRaw(u32 idx = 0)
		{
#if B3D_DEBUG
			B3D_ASSERT(sizeof(T) == ElementSize());
#endif
			return (T*)GetArrayAddrInternal(mInternal, sizeof(T), idx);
		}

		/**
		 * Creates a new array of managed objects.
		 *
		 * @tparam	T	ScriptObject wrapper for the specified managed type.
		 */
		template <class T>
		static ScriptArray Create(u32 size);

		/** Returns number of elements in the array. */
		u32 Size() const;

		/** Returns the size of an individual element in the array, in bytes. */
		u32 ElementSize() const;

		/** Returns the managed object representing this array. */
		MonoArray* GetInternal() const { return mInternal; }

		/** Returns the class of the elements within an array class. */
		static ::MonoClass* GetElementClass(::MonoClass* arrayClass);

		/** Returns the rank of the provided array class. */
		static u32 GetRank(::MonoClass* arrayClass);

		/** Builds an array class from the provided element class and a rank. */
		static ::MonoClass* BuildArrayClass(::MonoClass* elementClass, u32 rank);

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
		static u8* GetArrayAddrInternal(MonoArray* array, u32 size, u32 idx);

		/**
		 * Sets one or multiple entries from the array at the specified index, from raw memory. User must provide the size
		 * of the element, and it must match the element size expected by the array.
		 */
		static void SetArrayValInternal(MonoArray* array, u32 idx, const u8* value, u32 size, u32 count = 1);

		/**
		 * @}
		 */
	private:
		MonoArray* mInternal;
	};

	/** @} */

	/** @addtogroup Implementation-Internal
	 *  @{
	 */
	namespace Detail
	{
		// A layer of indirection for all methods specialized by ScriptArray. */

		template <class T>
		T ScriptArrayGet(MonoArray* array, u32 idx)
		{
			return *(T*)ScriptArray::GetArrayAddrInternal(array, sizeof(T), idx);
		}

		template <class T>
		void ScriptArraySet(MonoArray* array, u32 idx, const T& value)
		{
			ScriptArray::SetArrayValInternal(array, idx, (u8*)&value, sizeof(T));
		}

		template <>
		B3D_MONO_EXPORT String ScriptArrayGet(MonoArray* array, u32 idx);

		template <>
		B3D_MONO_EXPORT WString ScriptArrayGet(MonoArray* array, u32 idx);

		template <>
		B3D_MONO_EXPORT Path ScriptArrayGet(MonoArray* array, u32 idx);

		template <>
		B3D_MONO_EXPORT void ScriptArraySet<String>(MonoArray* array, u32 idx, const String& value);

		template <>
		B3D_MONO_EXPORT void ScriptArraySet<WString>(MonoArray* array, u32 idx, const WString& value);

		template <>
		B3D_MONO_EXPORT void ScriptArraySet<Path>(MonoArray* array, u32 idx, const Path& value);

		template <>
		B3D_MONO_EXPORT void ScriptArraySet<std::nullptr_t>(MonoArray* array, u32 idx, const std::nullptr_t& value);

		template <class T>
		inline ScriptArray ScriptArrayCreate(u32 size)
		{
			return ScriptArray(*T::GetMetaData()->ScriptClass, size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<u8>(u32 size)
		{
			return ScriptArray(MonoUtil::GetByteClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<i8>(u32 size)
		{
			return ScriptArray(MonoUtil::GetSByteClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<char>(u32 size)
		{
			return ScriptArray(MonoUtil::GetSByteClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<u16>(u32 size)
		{
			return ScriptArray(MonoUtil::GetUint16Class(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<i16>(u32 size)
		{
			return ScriptArray(MonoUtil::GetInt16Class(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<u32>(u32 size)
		{
			return ScriptArray(MonoUtil::GetUint32Class(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<i32>(u32 size)
		{
			return ScriptArray(MonoUtil::GetInt32Class(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<u64>(u32 size)
		{
			return ScriptArray(MonoUtil::GetUint64Class(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<i64>(u32 size)
		{
			return ScriptArray(MonoUtil::GetInt64Class(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<WString>(u32 size)
		{
			return ScriptArray(MonoUtil::GetStringClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<String>(u32 size)
		{
			return ScriptArray(MonoUtil::GetStringClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<Path>(u32 size)
		{
			return ScriptArray(MonoUtil::GetStringClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<float>(u32 size)
		{
			return ScriptArray(MonoUtil::GetFloatClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<double>(u32 size)
		{
			return ScriptArray(MonoUtil::GetDoubleClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<bool>(u32 size)
		{
			return ScriptArray(MonoUtil::GetBoolClass(), size);
		}

		template <>
		inline ScriptArray ScriptArrayCreate<MonoObject*>(u32 size)
		{
			return ScriptArray(MonoUtil::GetObjectClass(), size);
		}
	} // namespace Detail

	/** @} */

	template <class T>
	T ScriptArray::Get(u32 idx)
	{
		return Detail::ScriptArrayGet<T>(mInternal, idx);
	}

	/** Sets an entry from the array at the specified index. */
	template <class T>
	void ScriptArray::Set(u32 idx, const T& value)
	{
		Detail::ScriptArraySet<T>(mInternal, idx, value);
	}

	template <class T>
	ScriptArray ScriptArray::Create(u32 size)
	{
		return Detail::ScriptArrayCreate<T>(size);
	}
} // namespace b3d
