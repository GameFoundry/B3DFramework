//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	class GpuDevice;
	class IGpuCompletionTracker;
	class GpuFenceCompletionTracker;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Owns various GPU state for work on a single thread/fiber, and is used for submitting GPU work from that 
	 * thread/fiber. Backed by a GpuDevice; one device can have multiple contexts active at a time.
	 * 
	 * Provides parameter set and command buffer pools, transfer command buffers, transient memory allocations 
	 * and a completion tracker. 
	 * 
	 * It's expected that the render that will have one of these, and any worker operation will create its own context.
	 *
	 * @note	Not thread safe. A single thread/fiber must own and drive a given context.
	 */
	class B3D_EXPORT GpuWorkContext
	{
	public:
		/**
		 * Constructs a context that owns a timeline-fence completion tracker. Intended to be a
		 * short-lived RAII object driven entirely by an one off operation.
		 *
		 * @param	device	Device this context performs work against. Must outlive the context.
		 */
		explicit GpuWorkContext(GpuDevice& device);
		~GpuWorkContext();

		GpuWorkContext(const GpuWorkContext&) = delete;
		GpuWorkContext& operator=(const GpuWorkContext&) = delete;
		GpuWorkContext(GpuWorkContext&&) = delete;
		GpuWorkContext& operator=(GpuWorkContext&&) = delete;

		/** Device this context performs its GPU work against. */
		GpuDevice& GetDevice() const { return mDevice; }

		/** Completion tracker this context schedules object reclamation against. */
		IGpuCompletionTracker& GetCompletionTracker() const { return *mTracker; }

	private:
		friend class GpuDevice;

		/**
		 * Constructs a context that borrows the device's persistent completion tracker.
		 *
		 * @param	device			Device this context performs work against. Must outlive the context.
		 * @param	primaryTracker	Device-owned tracker the context borrows (not owned).
		 */
		GpuWorkContext(GpuDevice& device, IGpuCompletionTracker& primaryTracker);

		GpuDevice& mDevice; /**< Non-owning back-ref to the device. */
		IGpuCompletionTracker* mTracker; /**< Owned or borrowed. */
		TUnique<GpuFenceCompletionTracker> mOwnedTracker; /**< Non-null only owning a tracker. */
	};

	/** @} */

} // namespace b3d
