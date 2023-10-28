//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreThread.h"
#include "Threading/BsAsyncOp.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup CoreThread
		 *  @{
		 */

		/** Represents the current state of a RenderProxy. */
		enum class RenderProxyFlag
		{
			None = 0,
			Destroyed = 1 << 0, /**< Object has been destroyed and shouldn't be used. */
			ScheduledForInitialization = 1 << 1, /**< Object has been scheduled for initialization on the render thread, but the render thread hasn't processed it yet. */
			Initialized = 1 << 2, /**< Object's Initialize() method has been called. */
		};

		using RenderProxyFlags = Flags<RenderProxyFlag>;
		B3D_FLAGS_OPERATORS(RenderProxyFlag)

		/**
		 * Represents a version of a CoreObject that is meant to be used specifically on the render thread.
		 *
		 * @note	Render thread only.
		 */
		class B3D_CORE_EXPORT CoreObject
		{
		public:
			CoreObject();
			virtual ~CoreObject();

			/**	Called on the core thread when the object is first created. */
			virtual void Initialize();

			/**	Called on the core thread before the object is destroyed. */
			virtual void Destroy();

			/** Returns true if the object has been initialized. Non-initialized object should not be used. */
			bool IsInitialized() const { return mFlags.IsSet(RenderProxyFlag::Initialized); }

			/** Returns true if the object has been destroyed. Destroyed object should not be used. */
			bool IsDestroyed() const { return mFlags.IsSet(RenderProxyFlag::Destroyed); }

			/** Returns a shared pointer version of "this" pointer. */
			SPtr<CoreObject> GetShared() const { return mThis.lock(); }

		public: // ***** INTERNAL ******
			/** @name Internal
			 *  @{
			 */

			/**
			 * Sets a shared this pointer to this object. This MUST be called immediately after construction.
			 *
			 * @note	Called automatically by the factory creation methods so user should not call this manually.
			 */
			void SetShared(SPtr<CoreObject> sharedToThis);

			/** Called when the last reference in the shared pointer owning this object goes out of scope. */
			template <class T, class MemAlloc>
			static void SharedDeleter(CoreObject* object)
			{
				auto fnDestroy = [object]
				{
					if(!object->IsDestroyed())
						object->Destroy();

					B3DDelete<T, MemAlloc>((T*)object);
				};

				if(B3D_CURRENT_THREAD_ID != GetCoreThread().GetCoreThreadId())
					GetCoreThread().PostCommand(fnDestroy);
				else
					fnDestroy();
			}

			/** @} */

		protected:
			friend class CoreObjectManager;
			friend class bs::CoreObjectManager;
			friend class bs::CoreObject;

			/**
			 * Update internal data from provided memory buffer that was populated with data from the sim thread.
			 *
			 * @note
			 * This generally happens at the start of a core thread frame. Data used was recorded on the previous sim thread
			 * frame.
			 */
			virtual void SyncToCore(const CoreSyncData& data, FrameAllocator& allocator) {}

			/**
			 * Blocks the current thread until the resource is fully initialized.
			 *
			 * @note
			 * If you call this without calling initialize first a deadlock will occur. You should not call this from core thread.
			 */
			void Synchronize();

			RenderProxyFlags mFlags;
			std::weak_ptr<CoreObject> mThis;

			static Signal mCoreGpuObjectLoadedCondition;
			static Mutex mCoreGpuObjectLoadedMutex;
		};

		/** @} */
	} // namespace ct
} // namespace bs
