//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	class GpuFrameCapture;
	class GpuDevice;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Identifier for the HLSL source authored by BSL and consumed directly by the DirectX backends. Also the
	 * common source form that BSL passes are cleaned up into before being cross compiled to the other dialects.
	 */
	inline constexpr const char* kGpuProgramLanguageHlsl = "hlsl";

	/**
	 * Canonical identifier for the Vulkan-flavored GLSL dialect authored by BSL. Vulkan's native
	 * language identifier - the backend consumes these through its own glslang pipeline to emit
	 * SPIR-V.
	 */
	inline constexpr const char* kGpuProgramLanguageVksl = "vksl";

	/**
	 * Identifier for the MoltenVK-flavored VKSL dialect authored by BSL when running Vulkan on top of
	 * MoltenVK (the Vulkan backend on macOS). Same surface syntax as kGpuProgramLanguageVksl but with
	 * the @c METAL preprocessor define, so you can further customize for Metal specific code. 
	 */
	inline constexpr const char* kGpuProgramLanguageMvksl = "mvksl";

	/**
	 * Identifier for the Metal-flavored VKSL dialect authored by BSL when targeting the native Metal
	 * backend. Same surface syntax as kGpuProgramLanguageVksl but with the @c METAL preprocessor
	 * define, so you can further customize for Metal specific code.
	 */
	inline constexpr const char* kGpuProgramLanguageMsl = "msl";

	/**
	 * Identifier for the null shading language consumed by the null backend. Produces no actual shader code; used
	 * for headless or testing devices.
	 */
	inline constexpr const char* kGpuProgramLanguageNullsl = "nullsl";

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
		virtual TShared<GpuDevice> GetDevice(u32 index) const = 0;

		/************************************************************************/
		/* 								DEBUGGING/PROFILING						*/
		/************************************************************************/

		/** Captures all GPU commands following this point for analysis by an external tool (e.g. RenderDoc or nSight). */
		virtual void StartCapture();

		/** Stops capture started by StartCapture() and makes the captured commands ready for analysis. */
		virtual void StopCapture();

	protected:
		TShared<GpuFrameCapture> mFrameCapture;
	};

	/** @} */
} // namespace b3d
