//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObjectCore.h"
#include "Threading/BsAsyncOp.h"
#include "Utility/BsFlags.h"

namespace bs
{
	struct CoreSyncPacket;
	/** @addtogroup CoreThread
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
	class B3D_CORE_EXPORT CoreObject
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
		 * Blocks the current thread until the resource is fully initialized.
		 *
		 * @note
		 * If you call this without calling initialize first a deadlock will occur. You should not call this from core thread.
		 */
		void BlockUntilCoreInitialized() const;

		/** Returns an unique identifier for this object. */
		u64 GetInternalId() const { return mInternalID; }

		/** Returns a shared_ptr version of "this" pointer. */
		SPtr<CoreObject> GetShared() const { return mThis.lock(); }

		/**
		 * Returns an object that contains a core thread specific implementation of this CoreObject. Null is a valid return
		 * value in case object requires no core thread implementation.
		 *
		 * @note	Thread safe to retrieve, but its data is only valid on the core thread.
		 */
		SPtr<ct::CoreObject> GetCore() const { return mCoreSpecific; }

		/**
		 * Ensures all dirty syncable data is send to the core thread counterpart of this object (if any).
		 *
		 * @note	Call this if you have modified the object and need to make sure core thread has an up to date version.
		 *			Normally this is done automatically at the end of a frame.
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		void SyncToCore();

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
		template <class T, class MemAlloc>
		static void SharedDeleter(CoreObject* object)
		{
			if(!object->IsDestroyed())
				object->Destroy();

			B3DDelete<T, MemAlloc>((T*)object);
		}

		/** @} */
	protected:
		/**
		 * Constructs a new core object.
		 *
		 * @param[in]	requiresCoreInit	(optional) Determines if the ct::CoreObject counterpart of this object
		 *									(if it has any, see createCore()) requires initialization and destruction on the
		 *									core thread.
		 */
		CoreObject(bool requiresCoreInit = true);
		virtual ~CoreObject();

	private:
		friend class CoreObjectManager;

		CoreObjectFlags mFlags;
		u32 mCoreDirtyFlags;
		u64 mInternalID; // ID == 0 is not a valid ID
		std::weak_ptr<CoreObject> mThis;

	protected:
		/************************************************************************/
		/* 							CORE OBJECT SYNC                      		*/
		/************************************************************************/

		/**
		 * Creates an object that contains core thread specific data and methods for this CoreObject. Can be null if such
		 * object is not required.
		 */
		virtual SPtr<ct::CoreObject> CreateCore() const { return nullptr; }

		/**
		 * Marks the core data as dirty. This causes the syncToCore() method to trigger the next time objects are synced
		 * between core and sim threads.
		 *
		 * @param[in]	flags	(optional)	Flags in case you want to signal that only part of the internal data is dirty.
		 *									syncToCore() will be called regardless and it's up to the implementation to read
		 *									the flags value if needed.
		 */
		void MarkCoreDirty(u32 flags = 0xFFFFFFFF);

		/** Marks the core data as clean. Normally called right after syncToCore() has been called. */
		void MarkCoreClean() { mCoreDirtyFlags = 0; }

		/**
		 * Notifies the core object manager that this object is dependant on some other CoreObject(s), and the dependencies
		 * changed since the last call to this method. This will trigger a call to getCoreDependencies() to collect the
		 * new dependencies.
		 */
		void MarkDependenciesDirty();

		/**
		 * Checks is the core dirty flag set. This is used by external systems to know when internal data has changed and
		 * core thread potentially needs to be notified.
		 */
		bool IsCoreDirty() const { return mCoreDirtyFlags != 0; }

		/**
		 * Returns the exact value of the internal flag that signals whether an object needs to be synced with the core thread.
		 */
		u32 GetCoreDirtyFlags() const { return mCoreDirtyFlags; }

		/**
		 * Creates a data packet that will be used for syncing the core object with it's render thread counter-part.
		 * Caller must free the retrieved packet using the provided allocator, when done using it.
		 */
		virtual CoreSyncPacket* CreateSyncPacket(FrameAllocator& allocator, u32 flags) { return nullptr; }

		/**
		 * Populates the provided array with all core objects that this core object depends upon. Dependencies are required
		 * for syncing to the core thread, so the system can be aware to update the dependant objects if a dependency is
		 * marked as dirty (for example updating a camera's viewport should also trigger an update on camera so it has
		 * a chance to potentially update its data).
		 */
		virtual void GetCoreDependencies(Vector<CoreObject*>& dependencies) {}

		/**
		 * Gets called on an object when one of the dependencies (as returned from getCoreDependencies()) is marked as
		 * dirty. It gives the dependant object a chance to determine should it mark itself as dirty due to the dependency
		 * change. Dirty flags of the dependency object can be examined for more information on what part of the dependency
		 * was modified.
		 */
		virtual void OnDependencyDirty(CoreObject* dependency, u32 dirtyFlags)
		{
			// By default any changes on a dependency mark the parent dirty as well
			mCoreDirtyFlags |= kDirtyDependencyMask;
		}

	protected:
		SPtr<ct::CoreObject> mCoreSpecific;
	};

	/**
	 * Creates a new core object using the specified allocators and returns a shared pointer to it.
	 *
	 * @note
	 * All core thread object shared pointers must be created using this method or its overloads and you should not create
	 * them manually.
	 */
	template <class Type, class MainAlloc, class PtrDataAlloc, class... Args>
	SPtr<Type> B3DMakeCoreShared(Args&&... args)
	{
		return SPtr<Type>(B3DNew<Type, MainAlloc>(std::forward<Args>(args)...), &Type::template SharedDeleter<Type, MainAlloc>, StdAlloc<Type, PtrDataAlloc>());
	}

	/**
	 * Creates a new core object using the specified allocator and returns a shared pointer to it.
	 *
	 * @note
	 * All core thread object shared pointers must be created using this method or its overloads and you should not create
	 * them manually.
	 */
	template <class Type, class MainAlloc, class... Args>
	SPtr<Type> B3DMakeCoreShared(Args&&... args)
	{
		return SPtr<Type>(B3DNew<Type, MainAlloc>(std::forward<Args>(args)...), &Type::template SharedDeleter<Type, MainAlloc>, StdAlloc<Type, DefaultAllocatorTag>());
	}

	/**
	 * Creates a new core object and returns a shared pointer to it.
	 *
	 * @note
	 * All core thread object shared pointers must be created using this method or its overloads and you should not create
	 * them manually.
	 */
	template <class Type, class... Args>
	SPtr<Type> B3DMakeCoreShared(Args&&... args)
	{
		return SPtr<Type>(B3DNew<Type, DefaultAllocatorTag>(std::forward<Args>(args)...), &Type::template SharedDeleter<Type, DefaultAllocatorTag>, StdAlloc<Type, DefaultAllocatorTag>());
	}

	/**
	 * Creates a core object shared pointer using a previously constructed object.
	 *
	 * @note
	 * All core thread object shared pointers must be created using this method or its overloads and you should not create
	 * them manually.
	 */
	template <class Type, class MainAlloc = DefaultAllocatorTag, class PtrDataAlloc = DefaultAllocatorTag>
	SPtr<Type> B3DMakeCoreFromExisting(Type* data)
	{
		return SPtr<Type>(data, &Type::template SharedDeleter<Type, MainAlloc>, StdAlloc<Type, PtrDataAlloc>());
	}

	/** Returns associated core object, or null if the object is null. */
	template<class Type>
	auto B3DGetCoreObject(const SPtr<Type>& object)
	{
		return object == nullptr ? nullptr : object->GetCore();
	}

	/** Returns associated core object, or null if the object is null. */
	template<class Type>
	auto B3DGetCoreObject(const ResourceHandle<Type>& object)
	{
		return !object.IsLoaded() ? nullptr : object->GetCore();
	}


	/** @} */
} // namespace bs
