//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "CoreObject/B3DRenderProxy.h"
#include "Threading/B3DAsyncOp.h"
#include "Utility/B3DFlags.h"

namespace b3d
{
	struct RenderProxySyncPacket;

	/** @addtogroup CoreObject
	 *  @{
	 */

	/** Represents the current state of a CoreObject. */
	enum class CoreObjectFlag
	{
		None = 0,
		Destroyed = 1 << 0, /**< Object has been destroyed and shouldn't be used. */
		RequiresRenderProxy = 1 << 1, /**< Object creates a proxy representation for use by the render thread. */
		Initialized = 1 << 2, /**< Object's Initialize() method has been called. */
	};

	using CoreObjectFlags = Flags<CoreObjectFlag>;
	B3D_FLAGS_OPERATORS(CoreObjectFlag)

	/**
	 * Provides a standardized way to initialize/destroy objects, a unique runtime ID for each object, and a way to specify dependant CoreObject%s. 
	 * Optionally it may also be used to create render proxy objects for use by the render thread.
	 */
	class B3D_EXPORT CoreObject
	{
	public:
		/**
		 * Frees all the data held by this object.
		 *
		 * If the object has a render proxy, the internal reference to the render proxy will be released, but the
		 * proxy will not be destroyed unless this was the last reference. If render proxy destruction does happen, it
		 * is not immediate, but rather queued for destruction on the render thread.
		 */
		virtual void Destroy();

		/**
		 * Initializes all the internal data of this object. Must be called right after construction for new objects,
		 * or after deserialization for deserialized objects. If requested, render proxy is created and queued for
		 * initialization on the render thread.
		 */
		virtual void Initialize();

		/** Returns true if the object has been initialized. Non-initialized object should not be used. */
		bool IsInitialized() const { return mFlags.IsSet(CoreObjectFlag::Initialized); }

		/** Returns true if the object has been destroyed. Destroyed object should not be used. */
		bool IsDestroyed() const { return mFlags.IsSet(CoreObjectFlag::Destroyed); }

		/**
		 * Blocks the current thread until the render proxy is fully initialized on the render thread.
		 *
		 * @note
		 * If you call this without calling Initialize() first a deadlock will occur. You should not call this from the render thread.
		 */
		void BlockUntilRenderProxyInitialized() const;

		/** Returns an unique identifier for this object. */
		u64 GetInternalId() const { return mInternalID; }

		/** Returns a shared_ptr version of "this" pointer. */
		SPtr<CoreObject> GetShared() const { return mThis.lock(); }

		/**
		 * Returns an object that contains render thread specific implementation of this CoreObject. Null is a valid return
		 * value in case object requires no render thread implementation.
		 *
		 * @note	Thread safe to retrieve, but its data is only valid on the render thread.
		 */
		SPtr<render::RenderProxy> GetRenderProxy() const { return mRenderProxy; }

		/**
		 * Ensures all dirty syncable data is send to the render proxy (if any).
		 *
		 * @note	Call this if you have modified the object and need to make sure render thread has an up to date version.
		 *			Normally this is done automatically every frame.
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		void SyncToRenderProxy();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets a shared this pointer to this object. This must be called immediately after construction, but before
		 * Initialize().
		 *
		 * @note	This should be called by the factory creation methods so user doesn't have to call it manually.
		 */
		void SetShared(SPtr<CoreObject> ptrThis);

		/** Called when the last reference in the shared pointer owning this object goes out of scope. */
		template <class T, class AllocatorTag>
		static void SharedDeleter(CoreObject* object)
		{
			if(!object->IsDestroyed())
				object->Destroy();

			B3DDelete<T, AllocatorTag>((T*)object);
		}

		/** @} */
	protected:
		/**
		 * Constructs a new core object.
		 *
		 * @param	createRenderProxy		Set to true if the object requires a RenderProxy counter-part. Render proxies allow the object
		 *									to be used from the render thread, using data syncing from the main thread object to keep up to date.
		 *									If true, you class should override CreateRenderProxy() to create the proxy, and CreateRenderProxySyncPacket()
		 *									to provide the data for synchronization between the objects. MarkRenderProxyDataDirty() should be used to notify
		 *									the system data is dirty and render proxy needs updating.
		 */
		CoreObject(bool createRenderProxy = true);
		virtual ~CoreObject();

	private:
		friend class CoreObjectManager;

		CoreObjectFlags mFlags;
		u32 mRenderProxyDirtyFlags;
		u64 mInternalID; // ID == 0 is not a valid ID
		std::weak_ptr<CoreObject> mThis;

	protected:
		/**
		 * @note Render proxy
		 * @{
		 */

		/**
		 * Creates an object that contains render thread specific data and methods for this object. Can be null if such
		 * object is not required.
		 */
		virtual SPtr<render::RenderProxy> CreateRenderProxy() const { return nullptr; }

		/**
		 * Marks the render proxy data as dirty. This causes the SyncToRenderProxy() method to trigger the next time objects are synced
		 * to the render thread.
		 *
		 * @param	flags	Flags in case you want to signal that only part of the internal data is dirty.
		 *					SyncToRenderProxy() will be called regardless and it's up to the implementation to read
		 *					the flags value if needed.
		 */
		void MarkRenderProxyDataDirty(u32 flags = 0xFFFFFFFF);

		/** Marks the render proxy data as up to date. Normally called right after new data has been synced to the render thread. */
		void MarkRenderProxyDataUpToDate() { mRenderProxyDirtyFlags = 0; }

		/**
		 * Notifies the core object manager that this object is dependant on some other CoreObject(s), and the dependencies
		 * changed since the last call to this method. This will trigger a call to GetCoreDependencies() to collect the
		 * new dependencies.
		 */
		void MarkDependenciesDirty();

		/** Returns true if the render proxy has out of date data and required a new data sync to be up to date. */
		bool IsRenderProxyDataOutOfDate() const { return mRenderProxyDirtyFlags != 0; }

		/** Returns flags that specify which portion of the render proxy is out of date with this object. */
		u32 GetRenderProxyDirtyFlags() const { return mRenderProxyDirtyFlags; }

		/**
		 * Creates a data packet that will be used for syncing the core object with it's render proxy.
		 * Caller must free the retrieved packet using the provided allocator when done using it.
		 */
		virtual RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) { return nullptr; }

		/**
		 * Populates the provided array with all core objects that this core object depends upon. Dependencies are required
		 * for syncing to the render thread, so the system can be aware to update the dependant objects if a dependency is
		 * marked as dirty (for example updating a camera's viewport should also trigger an update on camera so it has
		 * a chance to potentially update its data).
		 */
		virtual void GetCoreDependencies(Vector<CoreObject*>& dependencies) {}

		/**
		 * Gets called on an object when one of the dependencies (as returned from GetCoreDependencies()) is marked as
		 * dirty. It gives the dependant object a chance to determine should it mark itself as dirty due to the dependency
		 * change. Dirty flags of the dependency object can be examined for more information on what part of the dependency
		 * was modified.
		 */
		virtual void OnDependencyDirty(CoreObject* dependency, u32 dirtyFlags)
		{
			// By default any changes on a dependency mark the parent dirty as well
			mRenderProxyDirtyFlags |= kDirtyDependencyMask;
		}

	protected:
		SPtr<render::RenderProxy> mRenderProxy;

		/** @} */
	};

	/** Returns associated render proxy object, or null if the object is null or has no render proxy. */
	template<class Type>
	SPtr<CoreVariantType<Type, true>> B3DGetRenderProxy(Type* const object)
	{
		return object == nullptr ? nullptr : std::static_pointer_cast<CoreVariantType<Type, true>>(object->GetRenderProxy());
	}

	/** Returns associated render proxy object, or null if the object is null or has no render proxy. */
	template<class Type>
	SPtr<CoreVariantType<Type, true>> B3DGetRenderProxy(const SPtr<Type>& object)
	{
		return object == nullptr ? nullptr : std::static_pointer_cast<CoreVariantType<Type, true>>(object->GetRenderProxy());
	}

	/** Returns associated render proxy object, or null if the object is null or has no render proxy. */
	template<class Type>
	SPtr<CoreVariantType<Type, true>> B3DGetRenderProxy(const SPtr<const Type>& object)
	{
		return object == nullptr ? nullptr : std::static_pointer_cast<CoreVariantType<Type, true>>(object->GetRenderProxy());
	}

	/** Returns associated render proxy object, or null if the object is null or has no render proxy. */
	template<class Type>
	WeakSPtr<CoreVariantType<Type, true>> B3DGetRenderProxy(const WeakSPtr<Type>& object)
	{
		SPtr<Type> strongObject = object.lock();
		return B3DGetRenderProxy(strongObject);
	}

	/** Returns associated render proxy object, or null if the object is null or has no render proxy. */
	template<class Type>
	WeakSPtr<CoreVariantType<Type, true>> B3DGetRenderProxy(const WeakSPtr<const Type>& object)
	{
		SPtr<const Type> strongObject = object.lock();
		return B3DGetRenderProxy(strongObject);
	}

	/** Returns associated render proxy object, or null if the object is null or has no render proxy. */
	template<class Type>
	SPtr<CoreVariantType<Type, true>> B3DGetRenderProxy(const TResourceHandle<Type>& object)
	{
		return !object.IsLoaded(false) ? nullptr : std::static_pointer_cast<CoreVariantType<Type, true>>(object->GetRenderProxy());
	}

	/** Returns associated render proxy object, or null if the object is null or has no render proxy. */
	template<class Type>
	SPtr<CoreVariantType<Type, true>> B3DGetRenderProxy(const TGameObjectHandle<Type>& object)
	{
		return !object.IsValid() ? nullptr : std::static_pointer_cast<CoreVariantType<Type, true>>(object->GetRenderProxy());
	}

	/** Returns a render proxy type associated with the provided type. */
	template <class T>
	struct RenderProxyTypeHelper
	{
		using Type = CoreVariantType<T, true>;
	};

	template <class T>
	using RenderProxyType = typename RenderProxyTypeHelper<T>::Type;

	/** @} */
} // namespace b3d
