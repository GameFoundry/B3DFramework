//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuBackend.h"

namespace b3d
{
	/** @addtogroup NullGpuBackend 
	 *  @{
	 */

	/**
	 * Handles initialization and shutdown of Null GPU backend, and provides access to GPU device objects.
	 *
	 * The Null backend is a no-op render API implementation used for headless rendering, automated testing,
	 * and continuous integration environments where no actual GPU is available or needed. All rendering
	 * operations are accepted but not executed, allowing the engine to run without GPU hardware.
	 */
	class NullGpuBackend : public GpuBackend
	{
		using Super = GpuBackend;
	public:
		void OnStartUp() override;
		void OnShutDown() override;

		u32 GetDeviceCount() const override { return (u32)mDevices.size(); }
		SPtr<GpuDevice> GetDevice(u32 index) const override { return mDevices[index]; }

	private:
		TInlineArray<SPtr<GpuDevice>, 1> mDevices;
	};

	/** Provides easy access to the NullGpuBackend. */
	NullGpuBackend& GetNullGpuBackend();

	/** @} */
} // namespace b3d
