//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsAsyncOp.h"

namespace bs
{
	namespace ct
	{
	/** @addtogroup CoreThread
	 *  @{
	 */

	/**
	 * Represents counterpart of a CoreObject that is meant to be used specifically on the core thread. 
	 *
	 * @note	Core thread only.
	 * @note	Different CoreObject implementations should implement this class for their own needs.
	 */
	class BS_CORE_EXPORT CoreObject
	{
	protected:
		/** Values that represent current state of the object */
		enum Flags
		{
			CGCO_INITIALIZED = 0x01, /**< Object has been initialized and can be used. */
			CGCO_SCHEDULED_FOR_INIT = 0x02 /**< Object has been scheduled for initialization but core thread has not completed it yet. */
		};

	public:
		CoreObject();
		virtual ~CoreObject();

		/**	Called on the core thread when the object is first created. */
		virtual void initialize();

		/** Returns a shared_ptr version of "this" pointer. */
		SPtr<CoreObject> getThisPtr() const { return mThis.lock(); }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets a shared this pointer to this object. This MUST be called immediately after construction.
		 *
		 * @note	Called automatically by the factory creation methods so user should not call this manually.
		 */
		void _setThisPtr(SPtr<CoreObject> ptrThis);

		/** @} */

	protected:
		friend class CoreObjectManager;
		friend class bs::CoreObject;

		/**
		 * Update internal data from provided memory buffer that was populated with data from the sim thread.
		 *
		 * @note	
		 * This generally happens at the start of a core thread frame. Data used was recorded on the previous sim thread 
		 * frame.
		 */
		virtual void syncToCore(const CoreSyncData& data) { }

		/**
		 * Blocks the current thread until the resource is fully initialized.
		 * 			
		 * @note	
		 * If you call this without calling initialize first a deadlock will occur. You should not call this from core thread.
		 */
		void synchronize();

		/**
		 * Returns true if the object has been properly initialized. Methods are not allowed to be called on the object
		 * until it is initialized.
		 */
		bool isInitialized() const { return (mFlags & CGCO_INITIALIZED) != 0; }
		bool isScheduledToBeInitialized() const { return (mFlags & CGCO_SCHEDULED_FOR_INIT) != 0; }

		void setIsInitialized(bool initialized) { mFlags = initialized ? mFlags | CGCO_INITIALIZED : mFlags & ~CGCO_INITIALIZED; }
		void setScheduledToBeInitialized(bool scheduled) { mFlags = scheduled ? mFlags | CGCO_SCHEDULED_FOR_INIT : mFlags & ~CGCO_SCHEDULED_FOR_INIT; }

		volatile UINT8 mFlags;
		std::weak_ptr<CoreObject> mThis;

		static Signal mCoreGpuObjectLoadedCondition;
		static Mutex mCoreGpuObjectLoadedMutex;
	};

	/** @} */
	}
}

