//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	class GpuFrameCapture;
	class GpuDevice;

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Provides access to all available GPU devices.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuBackend : public Module<GpuBackend>
	{
	public:
		~GpuBackend() override = default;

		virtual u32 GetDeviceCount() const = 0;
		virtual SPtr<GpuDevice> GetDevice(u32 index) const = 0;

		/************************************************************************/
		/* 								DEBUGGING/PROFILING						*/
		/************************************************************************/

		/** Captures all GPU commands following this point for analysis by an external tool (e.g. RenderDoc or nSight). */
		virtual void StartCapture();

		/** Stops capture started by StartCapture() and makes the captured commands ready for analysis. */
		virtual void StopCapture();

	protected:
		SPtr<GpuFrameCapture> mFrameCapture;
	};

	/** @} */
} // namespace b3d
