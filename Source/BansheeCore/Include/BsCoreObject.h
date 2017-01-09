//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObjectCore.h"
#include "BsAsyncOp.h"

namespace bs
{
	/** @addtogroup CoreThread
	 *  @{
	 */

	/**
	 * Core objects provides functionality for dealing with objects that need to exist on both simulation and core thread.
	 * It handles cross-thread initialization, destruction as well as syncing data between the two threads.
	 * 			
	 * It also provides a standardized way to initialize/destroy objects, and a way to specify dependant CoreObject%s. For
	 * those purposes it might also be used for objects that only exist on the core thread.
	 *
	 * @note	ct::CoreObject is a counterpart to CoreObject that is used exclusively on the core thread. CoreObject on the
	 *			other hand should be used exclusively on the simulation thread. Types that exist on both threads need to
	 *			implement both of these.
	 */
	class BS_CORE_EXPORT CoreObject
	{
	protected:
		/** Values that represent current state of the core object */
		enum Flags
		{
			CGO_DESTROYED = 0x01, /**< Object has been destroyed and shouldn't be used. */
			CGO_INIT_ON_CORE_THREAD = 0x02 /**< Object requires initialization on core thread. */
		};

	public:
		/**
		 * Frees all the data held by this object.
		 * 			
		 * @note	
		 * If this object require initialization on core thread destruction is not done immediately, and is 
		 * instead just scheduled on the core thread. Otherwise the object is destroyed immediately.
		 */
		virtual void destroy();

		/**
		 * Initializes all the internal resources of this object. Must be called right after construction. Generally you
		 * should call this from a factory method to avoid the issue where user forgets to call it.
		 * 					
		 * @note	
		 * If this object require initialization on core thread initialization is not done immediately, and is instead just 
		 * scheduled on the core thread. Otherwise the object is initialized immediately.
		 */
		virtual void initialize();

		/** Returns true if the object has been destroyed. Destroyed object should not be used. */
		bool isDestroyed() const { return (mFlags & CGO_DESTROYED) != 0; }

		/**
		 * Blocks the current thread until the resource is fully initialized.
		 * 			
		 * @note	
		 * If you call this without calling initialize first a deadlock will occur. You should not call this from core thread.
		 */
		void blockUntilCoreInitialized() const;

		/** Returns an unique identifier for this object. */
		UINT64 getInternalID() const { return mInternalID; }

		/** Returns a shared_ptr version of "this" pointer. */
		SPtr<CoreObject> getThisPtr() const { return mThis.lock(); }

		/**
		 * Returns an object that contains a core thread specific implementation of this CoreObject. Null is a valid return
		 * value in case object requires no core thread implementation.
		 *
		 * @note	Thread safe to retrieve, but its data is only valid on the core thread.
		 */
		SPtr<ct::CoreObject> getCore() const { return mCoreSpecific; }

		/**
		 * Ensures all dirty syncable data is send to the core thread counterpart of this object (if any).
		 *
		 * @note	Call this if you have modified the object and need to make sure core thread has an up to date version. 
		 *			Normally this is done automatically at the end of a frame.
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		void syncToCore();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets a shared this pointer to this object. This must be called immediately after construction, but before
		 * initialize().
		 *
		 * @note	This should be called by the factory creation methods so user doesn't have to call it manually.
		 */
		void _setThisPtr(SPtr<CoreObject> ptrThis);

		/** Schedules the object to be destroyed, and then deleted. */
		template<class T, class MemAlloc>
		static void _delete(CoreObject* obj)
		{
			if (!obj->isDestroyed())
				obj->destroy();

			bs_delete<T, MemAlloc>((T*)obj);
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

		/**
		 * Queues a command to be executed on the core thread, without a return value.
		 * 			
		 * @note	
		 * Requires a shared pointer to the object this function will be executed on, in order to make sure the object is 
		 * not deleted before the command executes. Can be null if the function is static or global.
		 */
		static void queueGpuCommand(const SPtr<ct::CoreObject>& obj, std::function<void()> func);

		/**
		 * Queues a command to be executed on the core thread, with a return value in the form of AsyncOp.
		 * 			
		 * @see		AsyncOp
		 * 			
		 * @note	
		 * Requires a shared pointer to the object this function will be executed on, in order to make sure the object is 
		 * not deleted before the command executes. Can be null if the function is static or global.
		 */
		static AsyncOp queueReturnGpuCommand(const SPtr<ct::CoreObject>& obj, std::function<void(AsyncOp&)> func);

		bool requiresInitOnCoreThread() const { return (mFlags & CGO_INIT_ON_CORE_THREAD) != 0; }
		void setIsDestroyed(bool destroyed) { mFlags = destroyed ? mFlags | CGO_DESTROYED : mFlags & ~CGO_DESTROYED; }
	private:
		friend class CoreObjectManager;

		volatile UINT8 mFlags;
		UINT32 mCoreDirtyFlags;
		UINT64 mInternalID; // ID == 0 is not a valid ID
		std::weak_ptr<CoreObject> mThis;

		/**
		 * Queues object initialization command on the core thread. The command is added to the primary core thread queue 
		 * and will be executed as soon as the core thread is ready.
		 */
		static void queueInitializeGpuCommand(const SPtr<ct::CoreObject>& obj);

		/**
		 * Queues object destruction command on the core thread. The command is added to the core thread queue of this 
		 * thread and will be executed after qzeze commands are submitted and any previously queued commands are executed.
		 *
		 * @note	It is up to the caller to ensure no other threads attempt to use this object.
		 */
		static void queueDestroyGpuCommand(const SPtr<ct::CoreObject>& obj);

		/** Helper wrapper method used for queuing commands with no return value on the core thread. */
		static void executeGpuCommand(const SPtr<ct::CoreObject>& obj, std::function<void()> func);

		/**	Helper wrapper method used for queuing commands with a return value on the core thread. */
		static void executeReturnGpuCommand(const SPtr<ct::CoreObject>& obj, std::function<void(AsyncOp&)> func, 
			AsyncOp& op);

	protected:
		/************************************************************************/
		/* 							CORE OBJECT SYNC                      		*/
		/************************************************************************/

		/**
		 * Creates an object that contains core thread specific data and methods for this CoreObject. Can be null if such 
		 * object is not required.
		 */
		virtual SPtr<ct::CoreObject> createCore() const { return nullptr; }

		/**
		 * Marks the core data as dirty. This causes the syncToCore() method to trigger the next time objects are synced 
		 * between core and sim threads.
		 *
		 * @param[in]	flags	(optional)	Flags in case you want to signal that only part of the internal data is dirty. 
		 *									syncToCore() will be called regardless and it's up to the implementation to read
		 *									the flags value if needed.
		 */
		void markCoreDirty(UINT32 flags = 0xFFFFFFFF);

		/** Marks the core data as clean. Normally called right after syncToCore() has been called. */
		void markCoreClean() { mCoreDirtyFlags = 0; }

		/**
		 * Notifies the core object manager that this object is dependant on some other CoreObject(s), and the dependencies
		 * changed since the last call to this method. This will trigger a call to getCoreDependencies() to collect the 
		 * new dependencies.
		 */
		void markDependenciesDirty();

		/**
		 * Checks is the core dirty flag set. This is used by external systems to know when internal data has changed and 
		 * core thread potentially needs to be notified.
		 */
		bool isCoreDirty() const { return mCoreDirtyFlags != 0; }

		/**
		 * Returns the exact value of the internal flag that signals whether an object needs to be synced with the core thread.
		 */
		UINT32 getCoreDirtyFlags() const { return mCoreDirtyFlags; }

		/**
		 * Copy internal dirty data to a memory buffer that will be used for updating core thread version of that data.
		 *
		 * @note	
		 * This generally happens at the end of every sim thread frame. Synced data becomes available to the core thread 
		 * the start of the next core thread frame.
		 */
		virtual CoreSyncData syncToCore(FrameAlloc* allocator) { return CoreSyncData(); }

		/**
		 * Populates the provided array with all core objects that this core object depends upon. Dependencies are required
		 * for syncing to the core thread, so the system can be aware to update the dependant objects if a dependency is
		 * marked as dirty (for example updating a camera's viewport should also trigger an update on camera so it has
		 * a chance to potentially update its data).
		 */
		virtual void getCoreDependencies(Vector<CoreObject*>& dependencies) { }

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
	template<class Type, class MainAlloc, class PtrDataAlloc, class... Args>
	SPtr<Type> bs_core_ptr_new(Args &&...args)
	{
		return SPtr<Type>(bs_new<Type, MainAlloc>(std::forward<Args>(args)...),
			&CoreObject::_delete<Type, MainAlloc>, StdAlloc<Type, PtrDataAlloc>());
	}

	/**
	 * Creates a new core object using the specified allocator and returns a shared pointer to it.
	 *
	 * @note	
	 * All core thread object shared pointers must be created using this method or its overloads and you should not create 
	 * them manually.
	 */
	template<class Type, class MainAlloc, class... Args>
	SPtr<Type> bs_core_ptr_new(Args &&...args)
	{
		return SPtr<Type>(bs_new<Type, MainAlloc>(std::forward<Args>(args)...),
			&CoreObject::_delete<Type, MainAlloc>, StdAlloc<Type, GenAlloc>());
	}

	/**
	 * Creates a new core object and returns a shared pointer to it.
	 *
	 * @note	
	 * All core thread object shared pointers must be created using this method or its overloads and you should not create 
	 * them manually.
	 */
	template<class Type, class... Args>
	SPtr<Type> bs_core_ptr_new(Args &&...args)
	{
		return SPtr<Type>(bs_new<Type, GenAlloc>(std::forward<Args>(args)...),
			&CoreObject::_delete<Type, GenAlloc>, StdAlloc<Type, GenAlloc>());
	}

	/**
	 * Creates a core object shared pointer using a previously constructed object.
	 *
	 * @note	
	 * All core thread object shared pointers must be created using this method or its overloads and you should not create 
	 * them manually.
	 */
	template<class Type, class MainAlloc = GenAlloc, class PtrDataAlloc = GenAlloc>
	SPtr<Type> bs_core_ptr(Type* data)
	{
		return SPtr<Type>(data, &CoreObject::_delete<Type, MainAlloc>, StdAlloc<Type, PtrDataAlloc>());  
	}

	/** @} */
}

