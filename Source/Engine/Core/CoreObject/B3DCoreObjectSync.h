//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"

namespace b3d
{
	/** @addtogroup Implementation-Internal
	 *  @{
	 */

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////// Various helper methods used for syncing data between the main and the render thread. /////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Checks is the provided type a resource handle
	template <typename T>
	struct B3DIsResourceHandle : std::false_type
	{};

	template <typename T>
	struct B3DIsResourceHandle<TResourceHandle<T>> : std::true_type
	{};

	// Returns the underlying type if the provided type is a resource handle, or itself otherwise
	template <typename T>
	struct B3DDecayResourceHandle
	{
		using value = T;
	};

	template <typename T>
	struct B3DDecayResourceHandle<TResourceHandle<T>>
	{
		using value = T;
	};

	template <typename T>
	using decay_all_t = typename B3DDecaySharedPointer<typename B3DDecayResourceHandle<std::decay_t<T>>::value>::value;

	// Converts a ResourceHandle to an underlying TShared, or if the type is not a ResourceHandle it just passes it
	// through as is.

	/** Pass non-resource-handle types as is. */
	template <class T>
	T&& RemoveHandle(T&& value, std::enable_if_t<!B3DIsResourceHandle<std::decay_t<T>>::value>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Convert a resource handle to the underlying resource TShared. */
	template <class T>
	decltype(((std::decay_t<T>*)nullptr)->GetShared()) RemoveHandle(T&& handle, std::enable_if_t<B3DIsResourceHandle<std::decay_t<T>>::value>* = 0)
	{
		if(handle.IsLoaded(false))
			return handle.GetShared();

		return nullptr;
	}

	/** @} */

	/** @addtogroup RenderThread
	 *  @{
	 */

	// Retrieves a RenderProxy from a CoreObject. If the type is not a core-object, it is just passed through as is.

	/** Pass non-shared-pointers as is, they aren't core objects. */
	template <class T>
	T&& GetRenderProxy(T&& value, std::enable_if_t<!B3DIsSharedPointer<std::decay_t<T>>::value && !B3DIsWeakSharedPointer<std::decay_t<T>>::value>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Pass shared-pointers to non-classes as is, they aren't core objects. */
	template <class T>
	T&& GetRenderProxy(T&& value, std::enable_if_t<(B3DIsSharedPointer<std::decay_t<T>>::value || B3DIsWeakSharedPointer<std::decay_t<T>>::value) && !std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Pass shared-pointers to classes that don't derive from CoreObject as is, they aren't core objects. */
	template <class T>
	T&& GetRenderProxy(T&& value, std::enable_if_t<(B3DIsSharedPointer<std::decay_t<T>>::value || B3DIsWeakSharedPointer<std::decay_t<T>>::value) && (std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value && !std::is_base_of<CoreObject, std::decay_t<typename std::decay_t<T>::element_type>>::value)>* = 0)
	{
		return std::forward<T>(value);
	}

	/** Convert shared-pointers with classes that derive from CoreObject to their RenderProxy variants. */
	template <class T>
	decltype(B3DGetRenderProxy(std::declval<T>()))
	GetRenderProxy(T&& value, std::enable_if_t<B3DIsSharedPointer<std::decay_t<T>>::value && (std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value && std::is_base_of<CoreObject, std::decay_t<typename std::decay_t<T>::element_type>>::value)>* = 0)
	{
		if(value)
			return B3DGetRenderProxy(value);

		return nullptr;
	}

	/** Convert shared-pointers with classes that derive from CoreObject to their RenderProxy variants. */
	template <class T>
	decltype(B3DGetRenderProxy(std::declval<T>()))
	GetRenderProxy(T&& value, std::enable_if_t<(B3DIsWeakSharedPointer<std::decay_t<T>>::value) && (std::is_class<std::decay_t<typename std::decay_t<T>::element_type>>::value && std::is_base_of<CoreObject, std::decay_t<typename std::decay_t<T>::element_type>>::value)>* = 0)
	{
		return B3DGetRenderProxy(value);
	}

	/** Packet containing data for synchronizing a CoreObject with its RenderProxy. */
	struct RenderProxySyncPacket
	{
		RenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
			: mAllocator(allocator), Flags(flags)
		{ }

		virtual ~RenderProxySyncPacket() = default;

		/** Transfers the data from this object into the provided RenderProxy. */
		virtual void ApplySyncData(void* object) { }

		/** Optional user-specified flags. */
		u32 Flags = 0;
		
	protected:
		FrameAllocator& mAllocator;
	};

	namespace implementation
	{
		/** Vector type that can be allocated using FrameAllocator, to be use for render proxy sync. */
		template <typename T, typename A = StdFrameAlloc<T>>
		using RenderProxySyncVector = std::vector<T, A>;

		/** Copies a field from the non-core object into the field in the RenderProxySyncPacket. */
		template <bool IsRenderProxy, class FieldTypeA, class FieldTypeB>
		void RenderProxySyncField(FieldTypeA& a, FieldTypeB& b, std::enable_if_t<!IsRenderProxy>* = 0)
		{
			a = GetRenderProxy(RemoveHandle(b));
		}

		/** Copies a field from the RenderProxySyncPacket to a core object. */
		template <bool IsRenderProxy, class FieldTypeA, class FieldTypeB>
		void RenderProxySyncField(FieldTypeA& a, FieldTypeB& b, std::enable_if_t<IsRenderProxy>* = 0)
		{
			b = a;
		}

		/** Copies an array from the non-core object into the field in the RenderProxySyncPacket. */
		template <bool IsRenderProxy, class FieldTypeA, class FieldTypeB>
		void RenderProxySyncField(RenderProxySyncVector<FieldTypeA>& a, Vector<FieldTypeB>& b, std::enable_if_t<!IsRenderProxy>* = 0)
		{
			a.resize(b.size());
			for(size_t index = 0; index < b.size(); ++index)
				a[index] = GetRenderProxy(RemoveHandle(b[index]));
		}

		/** Copies an array from the RenderProxySyncPacket to a core object. */
		template <bool IsRenderProxy, class FieldTypeA, class FieldTypeB>
		void RenderProxySyncField(RenderProxySyncVector<FieldTypeA>& a, Vector<FieldTypeB>& b, std::enable_if_t<IsRenderProxy>* = 0)
		{
			b.resize(a.size());
			for(size_t index = 0; index < a.size(); ++index)
				b[index] = std::move(a[index]);
		}

		/** Copies an array from the non-core object into the field in the RenderProxySyncPacket. */
		template <bool IsRenderProxy, class FieldTypeA, class FieldTypeB, u64 N>
		void RenderProxySyncField(RenderProxySyncVector<FieldTypeA>& a, TInlineArray<FieldTypeB, N>& b, std::enable_if_t<!IsRenderProxy>* = 0)
		{
			a.resize(b.size());
			for(size_t index = 0; index < b.size(); ++index)
				a[index] = GetRenderProxy(RemoveHandle(b[index]));
		}

		/** Copies an array from the RenderProxySyncPacket to a core object. */
		template <bool IsRenderProxy, class FieldTypeA, class FieldTypeB, u64 N>
		void RenderProxySyncField(RenderProxySyncVector<FieldTypeA>& a, TInlineArray<FieldTypeB, N>& b, std::enable_if_t<IsRenderProxy>* = 0)
		{
			b.resize(a.size());
			for(size_t index = 0; index < a.size(); ++index)
				b[index] = std::move(a[index]);
		}

		/** Defines an intermediate type used for storing data of type T in a RenderProxySyncPacket. */
		template <class T>
		struct RenderProxySyncPacketType
		{
			typedef decltype(GetRenderProxy(RemoveHandle(T()))) Type;
		};

		/** Defines an intermediate type used for storing data of type T in a RenderProxySyncPacket. */
		template <class T>
		struct RenderProxySyncPacketType<Vector<T>>
		{
			typedef RenderProxySyncVector<std::decay_t<decltype(GetRenderProxy(RemoveHandle(T())))>> Type;
		};

		/** Defines an intermediate type used for storing data of type T in a RenderProxySyncPacket. */
		template <class T, u64 N>
		struct RenderProxySyncPacketType<TInlineArray<T, N>>
		{
			typedef RenderProxySyncVector<std::decay_t<decltype(GetRenderProxy(RemoveHandle(T())))>> Type;
		};

		/** Initializes an intermediate type used for storing data of type T in a RenderProxySyncPacket with an allocator if needed. */
		template <class T>
		struct RenderProxySyncPacketTypeInitializeWithAllocator
		{
			static T Initialize(FrameAllocator& allocator)
			{
				return T();
			}
		};

		/** Initializes an intermediate type used for storing data of type T in a RenderProxySyncPacket with an allocator if needed. */
		template <class T>
		struct RenderProxySyncPacketTypeInitializeWithAllocator<RenderProxySyncVector<T>>
		{
			static RenderProxySyncVector<T> Initialize(FrameAllocator& allocator)
			{
				return RenderProxySyncVector<T>(&allocator);
			}
		};
	} // namespace implementation

	/**
	 * Begins the sync packet definition with explicit source and destination types. Use this when the destination type
	 * cannot be deduced via CoreVariantType (e.g. when using a custom proxy class instead of a CoreObject render-thread variant).
	 *
	 * @param SourceType	Type of the source (main-thread) object. Type must own the packet (the struct will be ClassType::Name).
	 * @param Name			Name of the packet structure.
	 * @param DestType		Type of the destination (render-thread) object.
	 */
#define B3D_SYNC_BLOCK_BEGIN_CUSTOM(SourceType, Name, DestType)                      \
	struct SourceType::Name : RenderProxySyncPacket                                  \
	{                                                                                \
		Name(SourceType& object, FrameAllocator& allocator, u32 flags = 0)           \
			: RenderProxySyncPacket(allocator, flags)                                \
		{                                                                            \
			PopulateSyncData(object);                                                \
		}                                                                            \
                                                                                     \
		~Name() override                                                             \
		{                                                                            \
			FreeEntries();                                                           \
		}                                                                            \
                                                                                     \
		typedef SourceType _Type;                                                    \
		typedef SourceType _SourceType;                                              \
		typedef DestType _DestType;                                                  \
                                                                                     \
		void ApplySyncData(void* object) override                                    \
		{                                                                            \
			ApplySyncData(*static_cast<_DestType*>(object));                         \
		}                                                                            \
                                                                                     \
	private:                                                                         \
		struct META_FirstEntry                                                       \
		{};                                                                          \
                                                                                     \
		void META_PopulateSyncDataPrevEntry(_SourceType& object, META_FirstEntry id) \
		{}                                                                           \
                                                                                     \
		void META_ApplySyncDataPrevEntry(_DestType& object, META_FirstEntry id)      \
		{}                                                                           \
                                                                                     \
		void META_FreePrevEntry(META_FirstEntry id)                                  \
		{}                                                                           \
                                                                                     \
		typedef META_FirstEntry

	/**
	 * Begins a new object sync packet definition. All specified entries will be gathered from the source object on construction of the packet,
	 * and can be applied to a destination object by calling ApplySyncData(). All packet data is internally allocated using a frame allocator.
	 *
	 * The packet definition will be created as part of @p ClassType, so you must declare `struct @p Name;` in your class.
	 *
	 * IMPORTANT: Type you pass to ApplySyncData() must be exactly ClassType as defined here. It cannot be a base type of ClassType or any other type otherwise
	 *			  you risk memory corruption due to the cast to void*.
	 *
	 * @param ClassType		Type of the source object from which the data will be gathered. Destination object type will be automatically deduced using
	 *						CoreVariantType<Type, Core> helper.
	 * @param Name			Name of the packet structure.
	 */
#define B3D_SYNC_BLOCK_BEGIN(ClassType, Name) B3D_SYNC_BLOCK_BEGIN_CUSTOM(ClassType, Name, RenderProxyType<ClassType>)

	/**
	 * Specifies an entry in the object sync packet definition. Both source and destination objects must have a field matching
	 * the entry name. The destination object type must be a friend of the source object type, so this object can access it's
	 * private fields.
	 *
	 * @param EntryName		Name of the field to sync.
	 */
#define B3D_SYNC_BLOCK_ENTRY(EntryName)                                                                                                    \
	META_Entry_##EntryName;                                                                                                                \
                                                                                                                                           \
	struct META_NextEntry_##EntryName                                                                                                      \
	{};                                                                                                                                    \
                                                                                                                                           \
	void META_PopulateSyncDataPrevEntry(_SourceType& object, META_NextEntry_##EntryName id)                                                \
	{                                                                                                                                      \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                                  \
		implementation::RenderProxySyncField<false>(EntryName, object.EntryName);                                                          \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_ApplySyncDataPrevEntry(_DestType& object, META_NextEntry_##EntryName id)                                                     \
	{                                                                                                                                      \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                                     \
		implementation::RenderProxySyncField<true>(EntryName, object.EntryName);                                                           \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                                 \
	{                                                                                                                                      \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                                      \
	}                                                                                                                                      \
                                                                                                                                           \
public:                                                                                                                                    \
	using Type##EntryName = std::decay_t<typename implementation::RenderProxySyncPacketType<decltype(_Type::EntryName)>::Type>;            \
	Type##EntryName EntryName = implementation::RenderProxySyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                           \
private:                                                                                                                                   \
	typedef META_NextEntry_##EntryName

	/**
	 * Equivalent to B3D_SYNC_BLOCK_ENTRY, but the caller must populate the packet field entry manually after construction. This is
	 * useful if a field exists in the destination object, but not in the source object.
	 *
	 * @param EntryType		Type of the field (always specify the non-core type).
	 * @param EntryName		Name of the field to sync.
	 */
#define B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(EntryType, EntryName)                                                                           \
	META_Entry_##EntryName;                                                                                                                \
                                                                                                                                           \
	struct META_NextEntry_##EntryName                                                                                                      \
	{};                                                                                                                                    \
                                                                                                                                           \
	void META_PopulateSyncDataPrevEntry(_SourceType& object, META_NextEntry_##EntryName id)                                                \
	{                                                                                                                                      \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                                  \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_ApplySyncDataPrevEntry(_DestType& object, META_NextEntry_##EntryName id)                                                     \
	{                                                                                                                                      \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                                     \
		implementation::RenderProxySyncField<true>(EntryName, object.EntryName);                                                           \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                                 \
	{                                                                                                                                      \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                                      \
	}                                                                                                                                      \
                                                                                                                                           \
public:                                                                                                                                    \
	using Type##EntryName = std::decay_t<typename implementation::RenderProxySyncPacketType<EntryType>::Type>;                             \
	Type##EntryName EntryName = implementation::RenderProxySyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                           \
private:                                                                                                                                   \
	typedef META_NextEntry_##EntryName

	/**
	 * Equivalent to B3D_SYNC_BLOCK_ENTRY, but the caller must manually read the packet field to apply it to the destination object.
	 * This is useful if a field exists in the source object, but not in the destination object.
	 *
	 * @param EntryType		Type of the field (always specify the non-core type).
	 * @param EntryName		Name of the field to sync.
	 */
#define B3D_SYNC_BLOCK_ENTRY_CUSTOM_GETTER(EntryType, EntryName)                                                                           \
	META_Entry_##EntryName;                                                                                                                \
                                                                                                                                           \
	struct META_NextEntry_##EntryName                                                                                                      \
	{};                                                                                                                                    \
                                                                                                                                           \
	void META_PopulateSyncDataPrevEntry(_SourceType& object, META_NextEntry_##EntryName id)                                                \
	{                                                                                                                                      \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                                  \
		implementation::RenderProxySyncField<false>(EntryName, object.EntryName);                                                          \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_ApplySyncDataPrevEntry(_DestType& object, META_NextEntry_##EntryName id)                                                     \
	{                                                                                                                                      \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                                     \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                                 \
	{                                                                                                                                      \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                                      \
	}                                                                                                                                      \
                                                                                                                                           \
public:                                                                                                                                    \
	using Type##EntryName = std::decay_t<typename implementation::RenderProxySyncPacketType<EntryType>::Type>;                             \
	Type##EntryName EntryName = implementation::RenderProxySyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                           \
private:                                                                                                                                   \
	typedef META_NextEntry_##EntryName

	/**
	 * Specifies an entry in the object sync packet definition. Note unlike with other B3D_SYNC_BLOCK_ENTRY* approaches, the field
	 * will not be automatically populated, nor automatically transferred to the destination object. Instead its fully up to the
	 * user to utilize the field as needed. This is useful if you just need to transfer some data, with either source or destination
	 * object having the field in its class.
	 *
	 * @param EntryType		Type of the field (always specify the non-core type).
	 * @param EntryName		Name of the field in the sync packet definition.
	 */
#define B3D_SYNC_BLOCK_ENTRY_CUSTOM(EntryType, EntryName)                                                                                  \
	META_Entry_##EntryName;                                                                                                                \
                                                                                                                                           \
	struct META_NextEntry_##EntryName                                                                                                      \
	{};                                                                                                                                    \
                                                                                                                                           \
	void META_PopulateSyncDataPrevEntry(_SourceType& object, META_NextEntry_##EntryName id)                                                \
	{                                                                                                                                      \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                                                  \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_ApplySyncDataPrevEntry(_DestType& object, META_NextEntry_##EntryName id)                                                     \
	{                                                                                                                                      \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                                                     \
	}                                                                                                                                      \
                                                                                                                                           \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                                                 \
	{                                                                                                                                      \
		META_FreePrevEntry(META_Entry_##EntryName());                                                                                      \
	}                                                                                                                                      \
                                                                                                                                           \
public:                                                                                                                                    \
	using Type##EntryName = std::decay_t<typename implementation::RenderProxySyncPacketType<EntryType>::Type>;                             \
	Type##EntryName EntryName = implementation::RenderProxySyncPacketTypeInitializeWithAllocator<Type##EntryName>::Initialize(mAllocator); \
                                                                                                                                           \
private:                                                                                                                                   \
	typedef META_NextEntry_##EntryName

	/**
	 * Specifies a base class sync packet to sync along this one. Note the user must manually construct and populate the child sync
	 * packet field, but the packet data will be applied automatically when the parent's data is applied. Child packet will
	 * also be automatically be destructed when the parent is destructed.
	 *
	 * @param ClassType		Base type of the object the packet is responsible for syncing.
	 * @param EntryName		Name of the field with the pointer to package.
	 */
#define B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(ClassType, EntryName)                                                \
	META_Entry_##EntryName;                                                                                   \
                                                                                                              \
	struct META_NextEntry_##EntryName                                                                         \
	{};                                                                                                       \
                                                                                                              \
	void META_PopulateSyncDataPrevEntry(_SourceType& object, META_NextEntry_##EntryName id) \
	{                                                                                                         \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                     \
	}                                                                                                         \
                                                                                                              \
	void META_ApplySyncDataPrevEntry(_DestType& object, META_NextEntry_##EntryName id)     \
	{                                                                                                         \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                        \
		if(EntryName) EntryName->ApplySyncData(&static_cast<CoreVariantType<ClassType, true>&>(object));      \
	}                                                                                                         \
                                                                                                              \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                    \
	{                                                                                                         \
		META_FreePrevEntry(META_Entry_##EntryName());                                                         \
		if(EntryName) mAllocator.Destruct(EntryName);                                                         \
	}                                                                                                         \
                                                                                                              \
public:                                                                                                       \
	RenderProxySyncPacket* EntryName = nullptr;                                                               \
                                                                                                              \
private:                                                                                                      \
	typedef META_NextEntry_##EntryName

	/**
	 * Specifies a class field that gets synced as its own packet. The child packet will be automatically constructed and populated
	 * on parent sync packet creation, and will be automatically applied when the parent is applied, as well as destructed when
	 * the parent is destructed.
	 *
	 * @param EntryName		Name of the field to sync.
	 * @param SyncPacketType	Type name of the sync packet of the child type. Must be part of the field's class.
	 */
#define B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(EntryName, SyncPacketType)                                                \
	META_Entry_##EntryName;                                                                                         \
                                                                                                                    \
	struct META_NextEntry_##EntryName                                                                               \
	{};                                                                                                             \
                                                                                                                    \
	void META_PopulateSyncDataPrevEntry(_SourceType& object, META_NextEntry_##EntryName id)       \
	{                                                                                                               \
		META_PopulateSyncDataPrevEntry(object, META_Entry_##EntryName());                                           \
		EntryName = mAllocator.Construct<decltype(_Type::EntryName)::SyncPacketType>(object.EntryName, mAllocator); \
	}                                                                                                               \
                                                                                                                    \
	void META_ApplySyncDataPrevEntry(_DestType& object, META_NextEntry_##EntryName id)           \
	{                                                                                                               \
		META_ApplySyncDataPrevEntry(object, META_Entry_##EntryName());                                              \
		if(EntryName) EntryName->ApplySyncData(&object.EntryName);                                                  \
	}                                                                                                               \
                                                                                                                    \
	void META_FreePrevEntry(META_NextEntry_##EntryName id)                                                          \
	{                                                                                                               \
		META_FreePrevEntry(META_Entry_##EntryName());                                                               \
		if(EntryName) mAllocator.Destruct(EntryName);                                                               \
	}                                                                                                               \
                                                                                                                    \
public:                                                                                                             \
	RenderProxySyncPacket* EntryName = nullptr;                                                                     \
                                                                                                                    \
private:                                                                                                            \
	typedef META_NextEntry_##EntryName

	/** Ends package definition started via B3D_SYNC_BLOCK_BEGIN. */
#define B3D_SYNC_BLOCK_END                                        \
	META_LastEntry;                                               \
                                                                  \
	void PopulateSyncData(_SourceType& object)  \
	{                                                             \
		META_PopulateSyncDataPrevEntry(object, META_LastEntry()); \
	}                                                             \
                                                                  \
	void ApplySyncData(_DestType& object)      \
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
} // namespace b3d
