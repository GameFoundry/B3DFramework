//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d::render
{
	class MetalGpuDevice;
	class MetalResourceManager;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Wraps a native Metal object. Tracks if the resource is bound on a command buffer or currently used on the GPU, and on which queues.
	 *
	 * @note Thread safe.
	 */
	template<class TBase>
	class TMetalResource : public TBase
	{
	public:
		static constexpr u32 kMaximumUniqueQueueCount = B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT;

		template<class... TBaseArgs>
		TMetalResource(MetalResourceManager* owner, TBaseArgs&&... baseArgs)
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
		MetalGpuDevice& GetDevice() const;

	protected:
		void OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags) override;
		void OnNotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags) override;

		/**
		 * Typed manager pointer. Shadows IGpuResource::mOwner so that subclasses calling mOwner->GetDevice()
		 * (and similar typed accessors on the manager) see the MetalResourceManager surface. The base's
		 * untyped pointer drives the deferred-destroy free path inside IGpuResource itself.
		 */
		MetalResourceManager* mOwner;

		u8 mReadUses[kMaximumUniqueQueueCount];
		u8 mWriteUses[kMaximumUniqueQueueCount];
	};

	/** Standard Metal resource with no specialized generic role. */
	using MetalResource = TMetalResource<IGpuResource>;

	/** @} */
} // namespace b3d::render
