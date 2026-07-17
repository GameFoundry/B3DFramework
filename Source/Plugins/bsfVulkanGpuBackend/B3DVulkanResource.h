//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Allocators/B3DStaticAlloc.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanResourceManager;

		/**
		 * Wraps a native Vulkan object. Extends a generic GPU resource base (@p TBase) with the
		 * Vulkan-specific portion of the lifetime state machine: queue ownership transitions and per-queue
		 * read/write use counters. Aggregate counters and deferred destruction are inherited from IGpuResource.
		 *
		 * @note Thread safe
		 */
		template<class TBase>
		class TVulkanResource : public TBase
		{
		protected:
			/** Possible states of this object. */
			enum class State
			{
				Normal,
				Shared
			};

		public:
			static constexpr u32 kMaximumUniqueQueueCount = B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT;

			template<class... TBaseArgs>
			TVulkanResource(VulkanResourceManager* owner, bool concurrency, TBaseArgs&&... baseArgs)
				: TBase(owner, std::forward<TBaseArgs>(baseArgs)...), mOwner(owner), mState(concurrency ? State::Shared : State::Normal)
			{}

			/** Returns the queue that currently owns the resource. Submit thread only. */
			bool GetOwnerQueueId(GpuQueueId& queueId) const
			{
				AssertIfNotSubmitThread();

				if(!mOwnerQueueValid)
					return false;

				queueId = mOwnerQueueId;
				return true;
			}

			/** Updates the queue that owns the resource. Submit thread only. */
			void SetOwnerQueueId(GpuQueueId queueId)
			{
				AssertIfNotSubmitThread();

				mOwnerQueueId = queueId;
				mOwnerQueueValid = true;
			}

			/** Returns true if the resource is only allowed to be used by a single queue family at once. */
			bool IsExclusive() const
			{
				Lock lock(this->mMutex);
				return mState != State::Shared;
			}

			/** Returns the device this resource is created on. */
			VulkanGpuDevice& GetDevice() const;

		protected:
			void OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags) override;

			/**
			 * Typed manager pointer. Shadows IGpuResource::mOwner so that subclasses calling mOwner->GetDevice()
			 * (and similar typed accessors on the manager) see the VulkanResourceManager surface. The base's untyped
			 * pointer drives the deferred-destroy free path inside IGpuResource itself.
			 */
			VulkanResourceManager* mOwner;

			GpuQueueId mOwnerQueueId;
			bool mOwnerQueueValid = false;
			State mState;

		};

		/** Standard Vulkan resource with no specialized generic role. */
		using VulkanResource = TVulkanResource<IGpuResource>;

		/**
		 * Creates and destroys VulkanResource%s on a single device.
		 *
		 * @note Thread safe
		 */
		class VulkanResourceManager : public GpuResourceManager
		{
		public:
			VulkanResourceManager(VulkanGpuDevice& device);

			/**
			 * Creates a new Vulkan resource of the specified type. User must call VulkanResource::Destroy() when done using
			 * the resource.
			 */
			template <class Type, class... Args>
			Type* Create(Args&&... args)
			{
				Type* resource = new(B3DAllocate(sizeof(Type))) Type(this, std::forward<Args>(args)...);
				RegisterResource(resource);
				return resource;
			}

			/** Returns the device that owns this manager. */
			VulkanGpuDevice& GetDevice() const;
		};

		/** @} */
	} // namespace render
} // namespace b3d
