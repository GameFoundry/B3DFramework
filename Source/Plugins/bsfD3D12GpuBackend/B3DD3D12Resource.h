//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d
{
	namespace render
	{
		class D3D12ResourceManager;

		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Wraps a native D3D12 object. Extends a generic GPU resource base (@p TBase) with the D3D12-specific portion
		 * of the lifetime state machine: per-queue read/write use counters. Aggregate counters and deferred
		 * destruction are inherited from IGpuResource.
		 *
		 * Unlike Vulkan there is no queue-family ownership machinery - D3D12 resources are usable on any queue
		 * without explicit ownership transfers.
		 *
		 * @note Thread safe
		 */
		template<class TBase>
		class TD3D12Resource : public TBase
		{
		public:
			static constexpr u32 kMaximumUniqueQueueCount = B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT;

			template<class... TBaseArgs>
			TD3D12Resource(D3D12ResourceManager* owner, TBaseArgs&&... baseArgs)
				: TBase(owner, std::forward<TBaseArgs>(baseArgs)...), mOwner(owner)
			{
				B3DZeroOut(mReadUses);
				B3DZeroOut(mWriteUses);
			}

			/**
			 * Returns a mask that has bits set for every queue that the resource is currently used (read or written) by.
			 *
			 * @param	useFlags	Flags for which to check use information (e.g. read only, write only, or both).
			 * @return				Bitmask of which queues is the resource used on.
			 */
			GpuQueueMask GetUseInfo(GpuAccessFlags useFlags) const;

			/** Returns the device this resource is created on. */
			D3D12GpuDevice& GetDevice() const;

		protected:
			void OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags) override;
			void OnNotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags) override;

			/**
			 * Typed manager pointer. Shadows IGpuResource::mOwner so that subclasses calling mOwner->GetDevice()
			 * (and similar typed accessors on the manager) see the D3D12ResourceManager surface. The base's untyped
			 * pointer drives the deferred-destroy free path inside IGpuResource itself.
			 */
			D3D12ResourceManager* mOwner;

			u8 mReadUses[kMaximumUniqueQueueCount];
			u8 mWriteUses[kMaximumUniqueQueueCount];
		};

		/** Standard D3D12 resource with no specialized generic role. */
		using D3D12Resource = TD3D12Resource<IGpuResource>;

		/** @} */
	} // namespace render
} // namespace b3d
