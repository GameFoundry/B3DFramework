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

	// Checks is the provided type a shared pointer
	template <typename T>
	struct is_shared_ptr : std::false_type
	{};

	template <typename T>
	struct is_shared_ptr<SPtr<T>> : std::true_type
	{};

	// Checks is the provided type a resource handle
	template <typename T>
	struct is_resource_handle : std::false_type
	{};

	template <typename T>
	struct is_resource_handle<ResourceHandle<T>> : std::true_type
	{};

	// Returns the underlying type if the provided type is a resource handle, or itself otherwise
	template <typename T>
	struct decay_handle
	{
		using value = T;
	};

	template <typename T>
	struct decay_handle<ResourceHandle<T>>
	{
		using value = T;
	};

	// Returns the underlying type if the provided type is a shared pointer, or itself otherwise
	template <typename T>
	struct decay_sptr
	{
		using value = T;
	};

	template <typename T>
	struct decay_sptr<SPtr<T>>
	{
		using value = typename SPtr<T>::element_type;
	};

	// Checks if a specific template specialization exists
	template <class T, std::size_t = sizeof(T)>
	std::true_type IsCompleteImpl(T*);
	std::false_type IsCompleteImpl(...);
	template <class T>
	using is_complete = decltype(is_complete_impl(std::declval<T*>()));

	template <typename T>
	using decay_all_t = typename decay_sptr<typename decay_handle<std::decay_t<T>>::value>::value;

	// Converts a ResourceHandle to an underlying SPtr, or if the type is not a ResourceHandle it just passes it
	// through as is.

	/** Pass non-resource-handle types as is. */
	template <class T>
	T&& RemoveHandle(T&& value, std::enable_if_t<!is_resource_handle<std::decay_t<T>>::value>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Convert a resource handle to the underlying resource SPtr. */
	template <class T>
	decltype(((std::decay_t<T>*)nullptr)->GetShared()) RemoveHandle(T&& handle, std::enable_if_t<is_resource_handle<std::decay_t<T>>::value>* = 0)
	{
		if(handle.IsLoaded(false))
			return handle.GetShared();

		return nullptr;
	}

	// Converts a sim thread CoreObject into a core thread CoreObject. If the type is not a core-object, it is just
	// passed through as is.

	/** Pass non-shared-pointers as is, they aren't core objects. */
	template <class T>
	T&& GetCoreObject(T&& value, std::enable_if_t<!is_shared_ptr<std::decay_t<T>>::value>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Pass shared-pointers to non-classes as is, they aren't core objects. */
	template <class T>
	T&& GetCoreObject(T&& value, std::enable_if_t<is_shared_ptr<std::decay_t<T>>::value && !std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Pass shared-pointers to classes that don't derive from CoreObject as is, they aren't core objects. */
	template <class T>
	T&& GetCoreObject(T&& value, std::enable_if_t<is_shared_ptr<std::decay_t<T>>::value && (std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value && !std::is_base_of<CoreObject, std::decay_t<typename std::decay_t<T>::element_type>>::value)>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Convert shared-pointers with classes that derive from CoreObject to their core thread variants. */
	template <class T>
	decltype(((std::decay_t<typename std::decay_t<T>::element_type>*)nullptr)->GetCore())
	GetCoreObject(T&& value, std::enable_if_t<is_shared_ptr<std::decay_t<T>>::value && (std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value && std::is_base_of<CoreObject, std::decay_t<typename std::decay_t<T>::element_type>>::value)>* = 0)
	{
		if(value)
			return value->GetCore();

		return nullptr;
	}

	/** @} */

	/** @addtogroup CoreThread
	 *  @{
	 */

	/**
	 * Writes the provided values into the underlying buffer using B3DRTTIWrite(). Each write advances the buffer to the
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
			: mStream(stream)
		{}

		/** If the type offers a rttiEnumFields method, recurse into it. */
		template <class T>
		void operator()(T&& value, std::enable_if_t<has_rttiEnumFields<T>::kValue>* = 0)
		{
			value.RttiEnumFields(*this);
		}

		/** If the type doesn't offer a rttiEnumFields method, perform the write using plain serialization. */
		template <class T>
		void operator()(T&& value, std::enable_if_t<!has_rttiEnumFields<T>::kValue>* = 0)
		{
			WriteInternal(GetCoreObject(RemoveHandle(std::forward<T>(value))));
		}

	private:
		template <class T>
		void WriteInternal(T&& value, std::enable_if_t<!is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			B3DRTTIWrite(value, mStream);
		}

		template <class T>
		void WriteInternal(T&& value, std::enable_if_t<is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;

			SPtrType* sptrPtr = new(mStream.Cursor()) SPtrType;
			*sptrPtr = (value);

			mStream.SkipBytes(sizeof(SPtrType));
		}

		Bitstream& mStream;
	};

	/**
	 * Reads values from the underlying buffer and writes them to the output object using B3DRTTIRead(). Each read advances
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
			: mStream(stream)
		{}

		/** If the type offers a rttiEnumFields method, recurse into it. */
		template <class T>
		void operator()(T&& value, std::enable_if_t<has_rttiEnumFields<T>::kValue>* = 0)
		{
			value.RttiEnumFields(*this);
		}

		/** If the type doesn't offer a rttiEnumFields method, perform the read using plain serialization. */
		template <class T>
		void operator()(T&& value, std::enable_if_t<!has_rttiEnumFields<T>::kValue>* = 0)
		{
			ReadInternal(std::forward<T>(value));
		}

	private:
		template <class T>
		void ReadInternal(T&& value, std::enable_if_t<!is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			B3DRTTIRead(value, mStream);
		}

		template <class T>
		void ReadInternal(T&& value, std::enable_if_t<is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			using SPtrType = std::decay_t<T>;

			SPtrType* sptr = (SPtrType*)(mStream.Cursor());
			value = *sptr;
			sptr->~SPtrType();

			mStream.SkipBytes(sizeof(SPtrType));
		}

		Bitstream& mStream;
	};

	/**
	 * Calculates size of provided values using B3DRTTISize(). All sizes are accumulated in the location provided upon
	 * construction.
	 *
	 * As input accepts any trivially copyable types, types with RTTIPlainType specializations, any shared pointers,
	 * as well as any resource handle or CoreObject shared ptr which will automatically get converted to
	 * their core thread variants.
	 *
	 * Types that provide a rttiEnumFields() method will have that method executed with this object provided as the
	 * parameter, allowing the type to recurse over its fields.
	 */
	struct RttiB3DCoreSyncSize
	{
		RttiB3DCoreSyncSize(uint32_t& size)
			: mSize(size)
		{}

		/** If the type offers a rttiEnumFields method, recurse into it. */
		template <class T>
		void operator()(T&& value, std::enable_if_t<has_rttiEnumFields<T>::kValue>* = 0)
		{
			value.RttiEnumFields(*this);
		}

		/** If the type doesn't offer a rttiEnumFields method, perform the read using plain serialization. */
		template <class T>
		void operator()(T&& value, std::enable_if_t<!has_rttiEnumFields<T>::kValue>* = 0)
		{
			GetSizeInternal(GetCoreObject(RemoveHandle(std::forward<T>(value))));
		}

	private:
		template <class T>
		void GetSizeInternal(T&& value, std::enable_if_t<!is_shared_ptr<std::decay_t<T>>::value>* = 0)
		{
			mSize += B3DRTTISize(value).Bytes;
		}

		template <class T>
		void GetSizeInternal(T&& value, std::enable_if_t<is_shared_ptr<std::decay_t<T>>::value>* = 0)
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
	template <class T>
	uint32_t CoreSyncGetSize(T& v)
	{
		uint32_t size = 0;
		v.RttiEnumFields(RttiB3DCoreSyncSize(size));
		return size;
	}

	/**
	 * Writes the provided object into the provided memory location, using rules for core object syncing. Advances the
	 * stream cursor by the number of bytes written. Object must have rttiEnumFields() method implementation.
	 */
	template <class T>
	void B3DCoreSyncWrite(T& v, Bitstream& stream)
	{
		v.RttiEnumFields(RttiCoreSyncWriter(stream));
	}

	/**
	 * Decodes information from the provided memory buffer and writes it into the provided object, using rules for core
	 * object syncing. Advances the stream cursor by number of bytes read. Object must have rttiEnumFields() method
	 * implementation.
	 */
	template <class T>
	void B3DCoreSyncRead(T& v, Bitstream& stream)
	{
		v.RttiEnumFields(RttiCoreSyncReader(stream));
	}

	/** Packet containing data for synchronizing a CoreObject with its render thread counter-part. */
	struct CoreSyncPacket
	{
		CoreSyncPacket(FrameAlloc& allocator)
			: mAllocator(allocator)
		{ }

		virtual ~CoreSyncPacket()
		{
			if(NextPacket)
			{
				mAllocator.Destruct(NextPacket);
			}
		}

		/** Next packet, in case multiple packets are created by a single core object. */
		CoreSyncPacket* NextPacket = nullptr;
		
	protected:
		FrameAlloc& mAllocator;
	};

	namespace implementation
	{
		/** Vector type that can be allocated using FrameAllocator, to be use for core object sync. */
		template <typename T, typename A = StdFrameAlloc<T>>
		using CoreSyncVector = std::vector<T, A>;

		/** Copies a field from the non-core object into the field in the CoreSyncPacket. */
		template <bool Core, class FieldTypeA, class FieldTypeB>
		void CoreSyncField(FieldTypeA&& a, FieldTypeB&& b, std::enable_if_t<!Core>* = 0)
		{
			a = GetCoreObject(RemoveHandle(b));
		}

		/** Copies a field from the CoreSyncPacket to a core object. */
		template <bool Core, class FieldTypeA, class FieldTypeB>
		void CoreSyncField(FieldTypeA&& a, FieldTypeB&& b, std::enable_if_t<Core>* = 0)
		{
			b = std::forward<FieldTypeA>(a);
		}

		/** Copies an array from the non-core object into the field in the CoreSyncPacket. */
		template <bool Core, class FieldTypeA, class FieldTypeB>
		void CoreSyncField(CoreSyncVector<FieldTypeA>& a, Vector<FieldTypeB>& b, std::enable_if_t<!Core>* = 0)
		{
			a.resize(b.size());
			for(size_t index = 0; index < b.size(); ++index)
				a[index] = GetCoreObject(RemoveHandle(b[index]));
		}

		/** Copies an array from the CoreSyncPacket to a core object. */
		template <bool Core, class FieldTypeA, class FieldTypeB>
		void CoreSyncField(CoreSyncVector<FieldTypeA>& a, Vector<FieldTypeB>& b, std::enable_if_t<Core>* = 0)
		{
			b.resize(a.size());
			for(size_t index = 0; index < a.size(); ++index)
				a[index] = std::move(b[index]);
		}

		/** Defines an intermediate type used for storing data of type T in a CoreSyncPacket. */
		template <class T>
		struct CoreSyncPacketType
		{
			typedef decltype(GetCoreObject(RemoveHandle(T()))) Type;
		};

		/** Defines an intermediate type used for storing data of type T in a CoreSyncPacket. */
		template <class T>
		struct CoreSyncPacketType<Vector<T>>
		{
			typedef CoreSyncVector<decltype(GetCoreObject(RemoveHandle(T())))> Type;
		};

		/** Initializes an intermediate type used for storing data of type T in a CoreSyncPacket with an allocator if needed. */
		template <class T>
		struct CoreSyncPacketTypeInitializeWithAllocator
		{
			static std::decay_t<T> Initialize(FrameAlloc& allocator)
			{
				return std::decay_t<T>();
			}
		};

		/** Initializes an intermediate type used for storing data of type T in a CoreSyncPacket with an allocator if needed. */
		template <class T>
		struct CoreSyncPacketTypeInitializeWithAllocator<CoreSyncVector<T>>
		{
			static CoreSyncVector<std::decay_t<T>> Initialize(FrameAlloc& allocator)
			{
				return CoreSyncVector<std::decay_t<T>>(&allocator);
			}
		};
	} // namespace implementation

#define B3D_SYNC_BLOCK_BEGIN(ClassType, Name)                                            \
	struct ClassType::Name : CoreSyncPacket                                              \
	{                                                                                    \
		Name(FrameAlloc& allocator)                                                      \
			: CoreSyncPacket(allocator)                                                  \
		{}                                                                               \
		typedef ClassType Type;                                                          \
		template <bool Core>                                                             \
		void Sync(CoreVariantType<Type, Core>& object)                                   \
		{                                                                                \
			SyncEntries<Core>(object);                                                   \
		}                                                                                \
                                                                                         \
	private:                                                                             \
		struct META_FirstEntry                                                           \
		{};                                                                              \
		template <bool Core>                                                             \
		void META_SyncPrevEntry(CoreVariantType<Type, Core>& object, META_FirstEntry id) \
		{}                                                                               \
                                                                                         \
		typedef META_FirstEntry

#define B3D_SYNC_BLOCK_ENTRY(EntryName)                                                                                            \
	META_Entry_##EntryName;                                                                                                        \
                                                                                                                                   \
	struct META_NextEntry_##EntryName                                                                                              \
	{};                                                                                                                            \
                                                                                                                                   \
	template <bool Core>                                                                                                           \
	void META_SyncPrevEntry(CoreVariantType<Type, Core>& object, META_NextEntry_##EntryName id)                                    \
	{                                                                                                                              \
		META_SyncPrevEntry<Core>(object, META_Entry_##EntryName());                                                                \
		implementation::CoreSyncField<Core>(EntryName, object.EntryName);                                                          \
	}                                                                                                                              \
                                                                                                                                   \
public:                                                                                                                            \
	using Type##EntryName = typename implementation::CoreSyncPacketType<decltype(Type::EntryName)>::Type;                          \
	Type##EntryName EntryName = implementation::CoreSyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                   \
private:                                                                                                                           \
	typedef META_NextEntry_##EntryName

#define B3D_SYNC_BLOCK_ENTRY_CUSTOM(EntryType, EntryName)                                                                           \
	META_Entry_##EntryName;                                                                                                         \
                                                                                                                                    \
	struct META_NextEntry_##EntryName                                                                                               \
	{};                                                                                                                             \
                                                                                                                                    \
	template <bool Core>                                                                                                            \
	void META_SyncPrevEntry(CoreVariantType<Type, Core>& object, META_NextEntry_##EntryName id)                                     \
	{                                                                                                                               \
		META_SyncPrevEntry<Core>(object, META_Entry_##EntryName());                                                                 \
	}                                                                                                                               \
                                                                                                                                    \
public:                                                                                                                             \
	using Type##EntryName = typename implementation::CoreSyncPacketType<EntryType>::Type;                                           \
	Type##EntryName EntryName = implementation::CoreSyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                    \
private:                                                                                                                            \
	typedef META_NextEntry_##EntryName

#define B3D_SYNC_BLOCK_END                                  \
	META_LastEntry;                                         \
                                                            \
	template <bool Core>                                    \
	void SyncEntries(CoreVariantType<Type, Core>& object)   \
	{                                                       \
		META_SyncPrevEntry<Core>(object, META_LastEntry()); \
	}                                                       \
	}                                                       \
	;

	/** @} */
} // namespace bs
