//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d
{
	class GpuCommandCapture;
	class GpuDevice;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Prefer using integrated GPU over discrete GPU when both are available. Shared by all backends - each defining its
	 * own copy would register duplicates, and only the first registered copy receives command line/config values.
	 */
	extern B3D_EXPORT TConfigVariable<bool> gGpuPreferIntegrated;

	/** Index of the GPU device to use. If negative, the best device is selected automatically. Shared by all backends. */
	extern B3D_EXPORT TConfigVariable<i32> gGpuPreferredDeviceIndex;

	/** Number of slots in the bindless resource heap, including the invalid slot. Clamped to the device limit. Shared by all backends. */
	extern B3D_EXPORT TConfigVariable<u32> gGpuBindlessResourceCapacity;

	/** Number of slots in the bindless sampler heap, including the invalid slot and the global samplers. Clamped to the device limit. Shared by all backends. */
	extern B3D_EXPORT TConfigVariable<u32> gGpuBindlessSamplerCapacity;

	/**
	 * Fraction of the bindless sampler heap that, once allocated, makes new sampler slot allocations first evict cached samplers that nothing
	 * else references. Shared by all backends.
	 */
	extern B3D_EXPORT TConfigVariable<float> gGpuBindlessSamplerEvictionThreshold;

#if B3D_BUILD_TYPE_DEVELOPMENT
	/**
	 * When enabled, backends log how long the driver takes to create each compute pipeline. With the driver's shader cache
	 * cleared this is the driver's shader compile time. Development builds only.
	 */
	extern B3D_EXPORT TConfigVariable<bool> gGpuDumpPipelineTimings;
#endif

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
	 * Identifier for native Metal Shading Language source consumed by the Metal backend. BSL authors
	 * this target in a Metal-flavored VKSL dialect (same surface syntax as kGpuProgramLanguageVksl but
	 * with the @c METAL preprocessor define) which is translated to MSL during cross-compilation;
	 * low-level rendering code provides MSL source directly.
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

		/** Returns the name of the running backend, which is the name of the plugin providing it (e.g. "bsfVulkanGpuBackend"). */
		virtual const char* GetBackendName() const = 0;

		virtual u32 GetDeviceCount() const = 0;
		virtual TShared<GpuDevice> GetDevice(u32 index) const = 0;

		/************************************************************************/
		/* 								DEBUGGING/PROFILING						*/
		/************************************************************************/

		/** Captures all GPU commands following this point for analysis by an external tool (e.g. RenderDoc or nSight). */
		virtual void StartGPUCommandCapture();

		/** Stops capture started by StartGPUCommandCapture() and makes the captured commands ready for analysis. */
		virtual void StopGPUCommandCapture();

	protected:
		TShared<GpuCommandCapture> mGPUCommandCapture;
	};

	/** @} */
} // namespace b3d
