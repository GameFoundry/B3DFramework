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

	/** Packet containing data for synchronizing a CoreObject with its render thread counter-part. */
	struct CoreSyncPacket
	{
		CoreSyncPacket(FrameAlloc& allocator, u32 flags)
			: mAllocator(allocator), Flags(flags)
		{ }

		virtual ~CoreSyncPacket() = default;

		/** Transfers the data from this object into the provided render thread variant of the core object. */
		virtual void ApplySyncData(void* object) { }

		/** Optional user-specified flags. */
		u32 Flags = 0;
		
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
		void CoreSyncField(FieldTypeA& a, FieldTypeB& b, std::enable_if_t<!Core>* = 0)
		{
			a = GetCoreObject(RemoveHandle(b));
		}

		/** Copies a field from the CoreSyncPacket to a core object. */
		template <bool Core, class FieldTypeA, class FieldTypeB>
		void CoreSyncField(FieldTypeA& a, FieldTypeB& b, std::enable_if_t<Core>* = 0)
		{
			b = a;
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
				b[index] = std::move(a[index]);
		}

		/** Copies an array from the non-core object into the field in the CoreSyncPacket. */
		template <bool Core, class FieldTypeA, class FieldTypeB, u64 N>
		void CoreSyncField(CoreSyncVector<FieldTypeA>& a, TInlineArray<FieldTypeB, N>& b, std::enable_if_t<!Core>* = 0)
		{
			a.resize(b.size());
			for(size_t index = 0; index < b.size(); ++index)
				a[index] = GetCoreObject(RemoveHandle(b[index]));
		}

		/** Copies an array from the CoreSyncPacket to a core object. */
		template <bool Core, class FieldTypeA, class FieldTypeB, u64 N>
		void CoreSyncField(CoreSyncVector<FieldTypeA>& a, TInlineArray<FieldTypeB, N>& b, std::enable_if_t<Core>* = 0)
		{
			b.resize(a.size());
			for(size_t index = 0; index < a.size(); ++index)
				b[index] = std::move(a[index]);
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
			typedef CoreSyncVector<std::decay_t<decltype(GetCoreObject(RemoveHandle(T())))>> Type;
		};

		/** Defines an intermediate type used for storing data of type T in a CoreSyncPacket. */
		template <class T, u64 N>
		struct CoreSyncPacketType<TInlineArray<T, N>>
		{
			typedef CoreSyncVector<std::decay_t<decltype(GetCoreObject(RemoveHandle(T())))>> Type;
		};

		/** Initializes an intermediate type used for storing data of type T in a CoreSyncPacket with an allocator if needed. */
		template <class T>
		struct CoreSyncPacketTypeInitializeWithAllocator
		{
			static T Initialize(FrameAlloc& allocator)
			{
				return T();
			}
		};

		/** Initializes an intermediate type used for storing data of type T in a CoreSyncPacket with an allocator if needed. */
		template <class T>
		struct CoreSyncPacketTypeInitializeWithAllocator<CoreSyncVector<T>>
		{
			static CoreSyncVector<T> Initialize(FrameAlloc& allocator)
			{
				return CoreSyncVector<T>(&allocator);
			}
		};
	} // namespace implementation

	/**
	 * Begins a new object sync packet definition. All specified entries will be gathered from the source object on construction of the packet,
	 * and can be applied to a destination object by calling ApplySyncData(). All packet data is internally allocated using a frame allocator.
	 *
	 * The packet definition will be created as part of @p ClassType, so you must declare `struct @p Name;` in your class.
	 *
	 * @param	ClassType		Type of the source object from which the data will be gathered. Destination object type will be automatically deduced using
	 *							CoreVariantType<Type, Core> helper.
	 * @param	Name			Name of the packet structure.
	 */
#define B3D_SYNC_BLOCK_BEGIN(ClassType, Name)                                                         \
	struct ClassType::Name : CoreSyncPacket                                                           \
	{                                                                                                 \
		Name(ClassType& object, FrameAlloc& allocator, u32 flags = 0)                                 \
			: CoreSyncPacket(allocator, flags)                                                        \
		{                                                                                             \
			PopulateSyncData(object);                                                                 \
		}                                                                                             \
                                                                                                      \
		~Name() override                                                                              \
		{                                                                                             \
			FreeEntries();                                                                            \
		}                                                                                             \
                                                                                                      \
		typedef ClassType _Type;                                                                      \
                                                                                                      \
		void ApplySyncData(void* object) override                                                     \
		{                                                                                             \
			ApplySyncData(*static_cast<CoreVariantType<_Type, true>*>(object));                       \
		}                                                                                             \
                                                                                                      \
	private:                                                                                          \
		struct META_FirstEntry                                                                        \
		{};                                                                                           \
                                                                                                      \
		void META_PopulateSyncDataPrevEntry(CoreVariantType<_Type, false>& object, META_FirstEntry id)\
		{}                                                                                            \
                                                                                                      \
		void META_ApplySyncDataPrevEntry(CoreVariantType<_Type, true>& object, META_FirstEntry id)    \
		{}                                                                                            \
                                                                                                      \
		void META_FreePrevEntry(META_FirstEntry id)                                                   \
		{}                                                                                            \
                                                                                                      \
		typedef META_FirstEntry

	/**
	 * Specifies an entry in the object sync packet definition. Both source and destination objects must have a field matching
	 * the entry name. The destination object type must be a friend of the source object type, so this object can access it's
	 * private fields.
	 *
	 * @param	EntryName		Name of the field to sync.
	 */
#define B3D_SYNC_BLOCK_ENTRY(EntryName)                                                                                             \
	META_Entry_##EntryName;                                                                                                         \
                                                                                                                                    \
	struct META_NextEntry_##EntryName                                                                                               \
	{};                                                                                                                             \
                                                                                                                                    \
	void META_PopulateSyncDataPrevEntry(CoreVariantType<_Type, false>& object, META_NextEntry_##EntryName id)                       \
	{                                                                                                                               \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                           \
		implementation::CoreSyncField<false>(EntryName, object.EntryName);                                                          \
	}                                                                                                                               \
                                                                                                                                    \
	void META_ApplySyncDataPrevEntry(CoreVariantType<_Type, true>& object, META_NextEntry_##EntryName id)                           \
	{                                                                                                                               \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                              \
		implementation::CoreSyncField<true>(EntryName, object.EntryName);                                                           \
	}                                                                                                                               \
                                                                                                                                    \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                          \
	{                                                                                                                               \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                               \
	}                                                                                                                               \
                                                                                                                                    \
public:                                                                                                                             \
	using Type##EntryName = std::decay_t<typename implementation::CoreSyncPacketType<decltype(_Type::EntryName)>::Type>;            \
	Type##EntryName EntryName = implementation::CoreSyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                    \
private:                                                                                                                            \
	typedef META_NextEntry_##EntryName

	/**
	 * Equivalent to B3D_SYNC_BLOCK_ENTRY, but the caller must populate the packet field entry manually after construction. This is
	 * useful if a field exists in the destination object, but not in the source object.
	 *
	 * @param	EntryType		Type of the field (always specify the non-core type).
	 * @param	EntryName		Name of the field to sync.
	 */
#define B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(EntryType, EntryName)                                                                    \
	META_Entry_##EntryName;                                                                                                         \
                                                                                                                                    \
	struct META_NextEntry_##EntryName                                                                                               \
	{};                                                                                                                             \
                                                                                                                                    \
	void META_PopulateSyncDataPrevEntry(CoreVariantType<_Type, false>& object, META_NextEntry_##EntryName id)                       \
	{                                                                                                                               \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                           \
	}                                                                                                                               \
                                                                                                                                    \
	void META_ApplySyncDataPrevEntry(CoreVariantType<_Type, true>& object, META_NextEntry_##EntryName id)                           \
	{                                                                                                                               \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                              \
		implementation::CoreSyncField<true>(EntryName, object.EntryName);                                                           \
	}                                                                                                                               \
                                                                                                                                    \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                          \
	{                                                                                                                               \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                               \
	}                                                                                                                               \
                                                                                                                                    \
public:                                                                                                                             \
	using Type##EntryName = std::decay_t<typename implementation::CoreSyncPacketType<EntryType>::Type>;                             \
	Type##EntryName EntryName = implementation::CoreSyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                    \
private:                                                                                                                            \
	typedef META_NextEntry_##EntryName

	/**
	 * Equivalent to B3D_SYNC_BLOCK_ENTRY, but the caller must manually read the packet field to apply it to the destination object.
	 * This is useful if a field exists in the source object, but not in the destination object.
	 *
	 * @param	EntryType		Type of the field (always specify the non-core type).
	 * @param	EntryName		Name of the field to sync.
	 */
#define B3D_SYNC_BLOCK_ENTRY_CUSTOM_GETTER(EntryType, EntryName)                                                                    \
	META_Entry_##EntryName;                                                                                                         \
                                                                                                                                    \
	struct META_NextEntry_##EntryName                                                                                               \
	{};                                                                                                                             \
                                                                                                                                    \
	void META_PopulateSyncDataPrevEntry(CoreVariantType<_Type, false>& object, META_NextEntry_##EntryName id)                       \
	{                                                                                                                               \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                           \
		implementation::CoreSyncField<false>(EntryName, object.EntryName);                                                          \
	}                                                                                                                               \
                                                                                                                                    \
	void META_ApplySyncDataPrevEntry(CoreVariantType<_Type, true>& object, META_NextEntry_##EntryName id)                           \
	{                                                                                                                               \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                              \
	}                                                                                                                               \
                                                                                                                                    \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                          \
	{                                                                                                                               \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                               \
	}                                                                                                                               \
                                                                                                                                    \
public:                                                                                                                             \
	using Type##EntryName = std::decay_t<typename implementation::CoreSyncPacketType<EntryType>::Type>;                             \
	Type##EntryName EntryName = implementation::CoreSyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                    \
private:                                                                                                                            \
	typedef META_NextEntry_##EntryName

	/**
	 * Specifies an entry in the object sync packet definition. Note unlike with other B3D_SYNC_BLOCK_ENTRY* approaches, the field
	 * will not be automatically populated, nor automatically transferred to the destination object. Instead its fully up to the
	 * user to utilize the field as needed. This is useful if you just need to transfer some data, with either source or destination
	 * object having the field in its class.
	 *
	 * @param	EntryType		Type of the field (always specify the non-core type).
	 * @param	EntryName		Name of the field in the sync packet definition.
	 */
#define B3D_SYNC_BLOCK_ENTRY_CUSTOM(EntryType, EntryName)                                                                           \
	META_Entry_##EntryName;                                                                                                         \
                                                                                                                                    \
	struct META_NextEntry_##EntryName                                                                                               \
	{};                                                                                                                             \
                                                                                                                                    \
	void META_PopulateSyncDataPrevEntry(CoreVariantType<_Type, false>& object, META_NextEntry_##EntryName id)                       \
	{                                                                                                                               \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                           \
	}                                                                                                                               \
                                                                                                                                    \
	void META_ApplySyncDataPrevEntry(CoreVariantType<_Type, true>& object, META_NextEntry_##EntryName id)                           \
	{                                                                                                                               \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                              \
	}                                                                                                                               \
                                                                                                                                    \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                          \
	{                                                                                                                               \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                               \
	}                                                                                                                               \
                                                                                                                                    \
public:                                                                                                                             \
	using Type##EntryName = std::decay_t<typename implementation::CoreSyncPacketType<EntryType>::Type>;                             \
	Type##EntryName EntryName = implementation::CoreSyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                    \
private:                                                                                                                            \
	typedef META_NextEntry_##EntryName

	/**
	 * Specifies a base class sync packet to sync along this one. Note the user must manually construct and populate the child sync
	 * packet field, but the packet data will be applied automatically when the parent's data is applied. Child packet will
	 * also be automatically be destructed when the parent is destructed.
	 *
	 * @param	ClassType		Base type of the object the packet is responsible for syncing. 
	 * @param	EntryName		Name of the field with the pointer to package.
	 *
	 */
#define B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(ClassType, EntryName)                                                    \
	META_Entry_##EntryName;                                                                                  \
                                                                                                             \
	struct META_NextEntry_##EntryName                                                                        \
	{};                                                                                                      \
                                                                                                             \
	void META_PopulateSyncDataPrevEntry(CoreVariantType<_Type, false>& object, META_NextEntry_##EntryName id)\
	{                                                                                                        \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                    \
	}                                                                                                        \
                                                                                                             \
	void META_ApplySyncDataPrevEntry(CoreVariantType<_Type, true>& object, META_NextEntry_##EntryName id)    \
	{                                                                                                        \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                       \
		if(EntryName) EntryName->ApplySyncData(&static_cast<CoreVariantType<ClassType, true>&>(object));     \
	}                                                                                                        \
                                                                                                             \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                   \
	{                                                                                                        \
		META_FreePrevEntry(META_Entry_##EntryName());                                                        \
		if(EntryName) mAllocator.Destruct(EntryName);                                                        \
	}                                                                                                        \
                                                                                                             \
public:                                                                                                      \
	CoreSyncPacket* EntryName = nullptr;                                                                     \
                                                                                                             \
private:                                                                                                     \
	typedef META_NextEntry_##EntryName

	/**
	 * Specifies a class field that gets synced as its own packet. The child packet will be automatically constructed and populated
	 * on parent sync packet creation, and will be automatically applied when the parent is applied, as well as destructed when
	 * the parent is destructed.
	 *
	 * @param	EntryName		Name of the field to sync.
	 * @param	SyncPacketType	Type name of the sync packet of the child type. Must be part of the field's class.
	 *
	 */
#define B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(EntryName, SyncPacketType)                                         \
	META_Entry_##EntryName;                                                                                  \
                                                                                                             \
	struct META_NextEntry_##EntryName                                                                        \
	{};                                                                                                      \
                                                                                                             \
	void META_PopulateSyncDataPrevEntry(CoreVariantType<_Type, false>& object, META_NextEntry_##EntryName id)\
	{                                                                                                        \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                    \
		EntryName = mAllocator.Construct<decltype(_Type::EntryName)::SyncPacketType>(object.EntryName, mAllocator); \
	}                                                                                                        \
                                                                                                             \
	void META_ApplySyncDataPrevEntry(CoreVariantType<_Type, true>& object, META_NextEntry_##EntryName id)    \
	{                                                                                                        \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                       \
		if(EntryName) EntryName->ApplySyncData(&object.EntryName);                                           \
	}                                                                                                        \
                                                                                                             \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                   \
	{                                                                                                        \
		META_FreePrevEntry(META_Entry_##EntryName());                                                        \
		if(EntryName) mAllocator.Destruct(EntryName);                                                        \
	}                                                                                                        \
                                                                                                             \
public:                                                                                                      \
	CoreSyncPacket* EntryName = nullptr;                                                                     \
                                                                                                             \
private:                                                                                                     \
	typedef META_NextEntry_##EntryName

	/** Ends package definition started via B3D_SYNC_BLOCK_BEGIN. */
#define B3D_SYNC_BLOCK_END                                        \
	META_LastEntry;                                               \
                                                                  \
	void PopulateSyncData(CoreVariantType<_Type, false>& object)  \
	{                                                             \
		META_PopulateSyncDataPrevEntry(object, META_LastEntry()); \
	}                                                             \
                                                                  \
	void ApplySyncData(CoreVariantType<_Type, true>& object)      \
	{                                                             \
		META_ApplySyncDataPrevEntry(object, META_LastEntry());    \
	}                                                             \
                                                                  \
	void FreeEntries()                                            \
	{                                                             \
		META_FreePrevEntry(META_LastEntry());                     \
	}                                                             \
	}                                                             \
	;

	/** @} */
} // namespace bs
