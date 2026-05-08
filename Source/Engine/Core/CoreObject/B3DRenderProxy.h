//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DRenderThread.h"
#include "Threading/B3DAsyncOp.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderThread 
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
		 * Represents a partial copy of a CoreObject,  meant to be used specifically on the render thread.
		 *
		 * @note	Render thread only.
		 */
		class B3D_EXPORT RenderProxy
		{
		public:
			RenderProxy();
			virtual ~RenderProxy();

			/**	Called on the render thread when the object is first created. */
			virtual void Initialize();

			/**	Called on the render thread before the object is destroyed. */
			virtual void Destroy();

			/** Returns true if the object has been initialized. Non-initialized object should not be used. */
			bool IsInitialized() const { return mFlags.IsSet(RenderProxyFlag::Initialized); }

			/** Returns true if the object has been destroyed. Destroyed object should not be used. */
			bool IsDestroyed() const { return mFlags.IsSet(RenderProxyFlag::Destroyed); }

			/** Returns a shared pointer version of "this" pointer. */
			TShared<RenderProxy> GetShared() const { return mThis.lock(); }

		public: // ***** INTERNAL ******
			/** @name Internal
			 *  @{
			 */

			/**
			 * Sets a shared this pointer to this object. This MUST be called immediately after construction.
			 *
			 * @note	Called automatically by the factory creation methods so user should not call this manually.
			 */
			void SetShared(TShared<RenderProxy> sharedToThis);

			/** Called when the last reference in the shared pointer owning this object goes out of scope. */
			template <class T, class AllocatorTag>
			static void SharedDeleter(RenderProxy* object)
			{
				auto fnDestroy = [object]
				{
					if(!object->IsDestroyed())
						object->Destroy();

					B3DDelete<T, AllocatorTag>((T*)object);
				};

				if(B3D_CURRENT_THREAD_ID != GetRenderThread().GetThreadId())
					GetRenderThread().PostCommand(fnDestroy, "RenderProxy::Destroy");
				else
					fnDestroy();
			}

			/** @} */

		protected:
			friend class CoreObjectManager;
			friend class b3d::CoreObjectManager;
			friend class b3d::CoreObject;

			/**
			 * Update internal data from provided memory buffer that was populated with data from the owning CoreObject.
			 *
			 * @note
			 * This generally happens at the start of a render thread frame. Data used was recorded on the previous main thread frame.
			 */
			virtual void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) {}

			/**
			 * Blocks the current thread until the resource is fully initialized.
			 *
			 * @note
			 * If you call this without calling initialize first a deadlock will occur. You should not call this from the render thread.
			 */
			void BlockUntilInitialized();

			RenderProxyFlags mFlags;
			std::weak_ptr<RenderProxy> mThis;

			static Signal mRenderProxyInitializedCondition;
			static Mutex mRenderProxyInitializedMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
