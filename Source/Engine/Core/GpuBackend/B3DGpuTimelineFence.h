//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Fence that may be used to be notified when GPU command buffer execution completes.
	 *
	 * @note Read-only methods are safe to call from any thread.
	 */
	class B3D_EXPORT GpuTimelineFence
	{
	public:
		virtual ~GpuTimelineFence() = default;

		/** Returns the latest value the GPU has signaled on this fence. */
		virtual u64 GetCompletedValue() const = 0;

		/** Returns true if @p value has been reached. */
		bool IsSignaled(u64 value) const { return value <= GetCompletedValue(); }

	protected:
		GpuTimelineFence() = default;

		GpuTimelineFence(const GpuTimelineFence&) = delete;
		GpuTimelineFence& operator=(const GpuTimelineFence&) = delete;
	};

	/** Timeline fence and value to signal. */
	struct GpuTimelineFenceAndValue
	{
		TShared<GpuTimelineFence> Fence;
		u64 Value = 0;
	};

	/** @} */
} // namespace b3d
