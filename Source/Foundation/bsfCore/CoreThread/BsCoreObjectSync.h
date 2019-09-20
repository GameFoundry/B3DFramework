//************************************ bs::framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"

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

		// Returns the underlying type if the provided type is a resource handle, or itself otherwise
		template<typename T> struct decay_handle { using value = T; };
		template<typename T> struct decay_handle<ResourceHandle<T>> { using value = T; };

		// Returns the underlying type if the provided type is a shared pointer, or itself otherwise
		template<typename T> struct decay_sptr { using value = T; };
		template<typename T> struct decay_sptr<SPtr<T>> { using value = typename SPtr<T>::element_type; };

		// Checks if a specific template specialization exists
		template <class T, std::size_t = sizeof(T)>
		std::true_type is_complete_impl(T *);
		std::false_type is_complete_impl(...);
		template <class T> using is_complete = decltype(is_complete_impl(std::declval<T*>()));

		template<typename T>
		using decay_all_t = typename decay_sptr<typename decay_handle<std::decay_t<T>>::value>::value;

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
	 * Writes the provided values into the underlying buffer using rtti_write(). Each write advances the buffer to the
	 * next write location. Caller is responsible for not writing out of range.
	 *
	 * As input accepts any trivially copyable types, types with RTTIPlainType specializations, any shared pointer, as well
	 * as any resource handle or CoreObject shared ptr which will automatically get converted to their core thread variants.
	 *
	 * Types that provide a rttiEnumFields() method will have that method executed with this object provided as the
	 * parameter, * allowing the type to recurse over its fields.
	 */
	struct RttiCoreSyncWriter
	{
		using MyType = RttiCoreSyncWriter;

		RttiCoreSyncWriter(Bitstream& stream)
			:mStream(stream)
		{}

		/** If the type offers a rttiEnumFields method, recurse into it. */
		template<class T>
		void operator()(T&& value, std::enable_if_t<has_rttiEnumFields<T>::value>* = 0)
		{
			value.rttiEnumFields(*this);
		}

		/** If the type doesn't offer a rttiEnumFields method, perform the write using plain serialization. */
		template<class T>
		void operator()(T&& value, std::enable_if_t<!has_rttiEnumFields<T>::value>* = 0)
		{
			writeInternal(detail::get_core_object(detail::remove_handle(std::forward<T>(value))));
		}

	private:
		template<class T>
		void writeInternal(T&& value, std::enable_if_t<
			!detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			rtti_write(value, mStream);
		}

		template<class T>
		void writeInternal(T&& value, std::enable_if_t<
			detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;

			SPtrType* sptrPtr = new (mStream.cursor()) SPtrType;
			*sptrPtr = (value);

			mStream.skipBytes(sizeof(SPtrType));
		}

		Bitstream& mStream;
	};

	/**
	 * Reads values from the underlying buffer and writes them to the output object using rtti_read(). Each read advances
	 * the buffer to the next value. Caller is responsible for not reading out of range.
	 *
	 * As output accepts any trivially copyable types, types with RTTIPlainType specializations and any shared pointers.
	 *
	 * Types that provide a rttiEnumFields() method will have that method executed with this object provided as the
	 * parameter, allowing the type to recurse over its fields.
	 */
	struct RttiCoreSyncReader
	{
		RttiCoreSyncReader(Bitstream& stream)
			:mStream(stream)
		{}

		/** If the type offers a rttiEnumFields method, recurse into it. */
		template<class T>
		void operator()(T&& value, std::enable_if_t<has_rttiEnumFields<T>::value>* = 0)
		{
			value.rttiEnumFields(*this);
		}

		/** If the type doesn't offer a rttiEnumFields method, perform the read using plain serialization. */
		template<class T>
		void operator()(T&& value, std::enable_if_t<!has_rttiEnumFields<T>::value>* = 0)
		{
			readInternal(std::forward<T>(value));
		}

	private:
		template<class T>
		void readInternal(T&& value, std::enable_if_t<
			!detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			rtti_read(value, mStream);
		}

		template<class T>
		void readInternal(T&& value, std::enable_if_t<
			detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;

			SPtrType* sptr = (SPtrType*)(mStream.cursor());
			value = *sptr;
			sptr->~SPtrType();

			mStream.skipBytes(sizeof(SPtrType));
		}

		Bitstream& mStream;
	};

	/**
	 * Calculates size of provided values using rtti_size(). All sizes are accumulated in the location provided upon
	 * construction.
	 *
	 * As input accepts any trivially copyable types, types with RTTIPlainType specializations, any shared pointers,
	 * as well as any resource handle or CoreObject shared ptr which will automatically get converted to
	 * their core thread variants.
	 *
	 * Types that provide a rttiEnumFields() method will have that method executed with this object provided as the
	 * parameter, allowing the type to recurse over its fields.
	 */
	struct RttiCoreSyncSize
	{
		RttiCoreSyncSize(uint32_t& size)
			:mSize(size)
		{ }

		/** If the type offers a rttiEnumFields method, recurse into it. */
		template<class T>
		void operator()(T&& value, std::enable_if_t<has_rttiEnumFields<T>::value>* = 0)
		{
			value.rttiEnumFields(*this);
		}

		/** If the type doesn't offer a rttiEnumFields method, perform the read using plain serialization. */
		template<class T>
		void operator()(T&& value, std::enable_if_t<!has_rttiEnumFields<T>::value>* = 0)
		{
			getSizeInternal(detail::get_core_object(detail::remove_handle(std::forward<T>(value))));
		}

	private:
		template<class T>
		void getSizeInternal(T&& value, std::enable_if_t<
			!detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			mSize += rtti_size(value).bytes;
		}

		template<class T>
		void getSizeInternal(T&& value, std::enable_if_t<
			detail::is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;
			mSize += sizeof(SPtrType);
		}

		uint32_t& mSize;
	};

	/**
	 * Calculates the size of the provided object, using rules for core object syncing. Object must have rttiEnumFields()
	 * method implementation.
	 */
	template<class T>
	uint32_t csync_size(T& v)
	{
		uint32_t size = 0;
		v.rttiEnumFields(RttiCoreSyncSize(size));
		return size;
	}

	/**
	 * Writes the provided object into the provided memory location, using rules for core object syncing. Advances the
	 * stream cursor by the number of bytes written. Object must have rttiEnumFields() method implementation.
	 */
	template<class T>
	void csync_write(T& v, Bitstream& stream)
	{
		v.rttiEnumFields(RttiCoreSyncWriter(stream));
	}

	/**
	 * Decodes information from the provided memory buffer and writes it into the provided object, using rules for core
	 * object syncing. Advances the stream cursor by number of bytes read. Object must have rttiEnumFields() method
	 * implementation.  
	 */
	template<class T>
	void csync_read(T& v, Bitstream& stream)
	{
		v.rttiEnumFields(RttiCoreSyncReader(stream));
	}

	/** @} */
}

