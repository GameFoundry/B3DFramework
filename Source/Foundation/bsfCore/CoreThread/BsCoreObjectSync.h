//************************************ bs::framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////// Various helper methods used for syncing data between the simulation and the core threads. ///////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	namespace detail
	{
		// Checks is the provided type a shared pointer
		template<typename T> struct is_shared_ptr : std::false_type {};
		template<typename T> struct is_shared_ptr<SPtr<T>> : std::true_type {};

		// Checks is the provided type a resource handle
		template<typename T> struct is_resource_handle : std::false_type {};
		template<typename T> struct is_resource_handle<ResourceHandle<T>> : std::true_type {};

		// Converts a ResourceHandle to an underlying SPtr, or if the type is not a ResourceHandle it just passes it
		// through as is.

		/** Pass non-resource-handle types as is. */
		template <class T>
		T&& remove_handle(T&& value, std::enable_if_t<
			!is_resource_handle<std::decay_t<T>>::value>* = 0)
		{
			return std::forward<T>(value);
		}

		/** Convert a resource handle to the underlying resource SPtr. */
		template <class T>
		decltype(((std::decay_t<T>*)nullptr)->getInternalPtr()) remove_handle(T&& handle, std::enable_if_t<
			is_resource_handle<std::decay_t<T>>::value>* = 0)
		{
			if(handle.isLoaded(false))
				return handle.getInternalPtr();

			return nullptr;
		}

		// Converts a sim thread CoreObject into a core thread CoreObject. If the type is not a core-object, it is just
		// passed through as is.

		/** Pass non-shared-pointers as is, they aren't core objects. */
		template <class T>
		T&& get_core_object(T&& value, std::enable_if_t<
			!is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			return std::forward<T>(value);
		}

		/** Pass shared-pointers to non-classes as is, they aren't core objects. */
		template <class T>
		T&& get_core_object(T&& value, std::enable_if_t<
			is_shared_ptr<std::decay_t<T>>::value &&
			!std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value>* = 0)
		{
			return std::forward<T>(value);
		}

		/** Pass shared-pointers to classes that don't derive from CoreObject as is, they aren't core objects. */
		template <class T>
		T&& get_core_object(T&& value, std::enable_if_t<
			is_shared_ptr<std::decay_t<T>>::value &&
			(std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value &&
				!std::is_base_of<CoreObject, std::decay_t<typename std::decay_t<T>::element_type>>::value)>* = 0)
		{
			return std::forward<T>(value);
		}

		/** Convert shared-pointers with classes that derive from CoreObject to their core thread variants. */
		template <class T>
		decltype(((std::decay_t<typename std::decay_t<T>::element_type>*)nullptr)->getCore()) 
		get_core_object(T&& value, std::enable_if_t<
			is_shared_ptr<std::decay_t<T>>::value &&
			(std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value &&
				std::is_base_of<CoreObject, std::decay_t<typename std::decay_t<T>::element_type>>::value)>* = 0)
		{
			if(value)
				return value->getCore();

			return nullptr;
		}
	}

	/** @} */

	/** @addtogroup CoreThread
	 *  @{
	 */

	/** 
	 * Overloads operator << and writes the provided values into the underlying buffer using rttiWriteElem(). Each write
	 * advances the buffer to the next write location. Caller is responsible for not writing out of range.
	 * 
	 * As input accepts any trivially copyable types, types with RTTIPlainType specializations, any shared pointer, as well
	 * as any resource handle or CoreObject shared ptr which will automatically get converted to their core thread variants.
	 */
	struct RttiCoreSyncWriter
	{
		RttiCoreSyncWriter(char** data)
			:mWritePtr(data)
		{}

		template<class T>
		friend RttiCoreSyncWriter& operator<<(RttiCoreSyncWriter&, T&&);

	private:
		template<class T>
		void writeInternal(T&& value, std::enable_if_t<
			!detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			(*mWritePtr) = rttiWriteElem(value, *mWritePtr);
		}

		template<class T>
		void writeInternal(T&& value, std::enable_if_t<
			detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;

			SPtrType* sptrPtr = new (*mWritePtr) SPtrType;
			*sptrPtr = (value);

			(*mWritePtr) += sizeof(SPtrType);
		}

		char** mWritePtr;
	};

	template<class T>
	RttiCoreSyncWriter& operator<<(RttiCoreSyncWriter& writer, T&& value)
	{
		writer.writeInternal(detail::get_core_object(detail::remove_handle(std::forward<T>(value))));
		return writer;
	}

	/** 
	 * Overloads operator << and reads values from the underlying buffer using rttiReadElem(). Each read advances the buffer
	 * to the next value. Caller is responsible for not reading out of range.
	 *
	 * As output accepts any trivially copyable types, types with RTTIPlainType specializations and any shared pointers.
	 */
	struct RttiCoreSyncReader
	{
		RttiCoreSyncReader(char** data)
			:mReadPtr(data)
		{}

		template<class T>
		friend RttiCoreSyncReader& operator<<(RttiCoreSyncReader&, T&&);
	private:
		template<class T>
		void readInternal(T&& value, std::enable_if_t<
			!detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			(*mReadPtr) = rttiReadElem(value, *mReadPtr);
		}

		template<class T>
		void readInternal(T&& value, std::enable_if_t<
			detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;

			SPtrType* sptr = (SPtrType*)(*mReadPtr);
			value = *sptr;
			sptr->~SPtrType();

			(*mReadPtr) += sizeof(SPtrType);
		}

		char** mReadPtr;
	};

	template<class T>
	RttiCoreSyncReader& operator<<(RttiCoreSyncReader& reader, T&& value)
	{
		reader.readInternal(value);
		return reader;
	}

	/** 
	 * Overloads operator << and calculates size of provided values using rttiGetElemSize(). All sizes are accumulated in
	 * the location provided upon construction.
	 *
	 * As input accepts any trivially copyable types, types with RTTIPlainType specializations, any shared pointer, as well
	 * as any resource handle or CoreObject shared ptr which will automatically get converted to their core thread variants.
	 */
	struct RttiCoreSyncSize
	{
		RttiCoreSyncSize(UINT32& size)
			:mSize(size)
		{ }

		template<class T>
		friend RttiCoreSyncSize& operator<< (RttiCoreSyncSize&, T&&);
	private:
		template<class T>
		void getSizeInternal(T&& value, std::enable_if_t<
			!detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			mSize += rttiGetElemSize(value);
		}

		template<class T>
		void getSizeInternal(T&& value, std::enable_if_t<
			detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;
			mSize += sizeof(SPtrType);
		}

		UINT32& mSize;
	};

	template<class T>
	RttiCoreSyncSize& operator<< (RttiCoreSyncSize& sizer, T&& value)
	{
		sizer.getSizeInternal(detail::get_core_object(detail::remove_handle(std::forward<T>(value))));
		return sizer;
	}

	/** @} */
}

