//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuBackend.h"

namespace b3d
{
	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Handles initialization and shutdown of Metal GPU backend, and provides access to GPU device objects.
	 *
	 * Implements the Metal (Apple) render API used on macOS and iOS. Currently exposes a single default
	 * device, selected via MTLCreateSystemDefaultDevice equivalent; multi-GPU support may be added later.
	 */
	class MetalGpuBackend : public GpuBackend
	{
		using Super = GpuBackend;
	public:
		void OnStartUp() override;
		void OnShutDown() override;

		u32 GetDeviceCount() const override { return (u32)mDevices.size(); }
		TShared<GpuDevice> GetDevice(u32 index) const override
		{
			B3D_ASSERT(index < mDevices.size());
			return mDevices[index];
		}

	private:
		TInlineArray<TShared<GpuDevice>, 1> mDevices;
	};

	/** Provides easy access to the MetalGpuBackend. */
	MetalGpuBackend& GetMetalGpuBackend();

	/** @} */
} // namespace b3d
