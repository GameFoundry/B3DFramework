//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuProgram.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a GPU program.
		 *
		 * Loads an offline-compiled @c metallib and resolves the requested entry point as an @c MTLFunction.
		 * Both handles are held in an ObjC++-only pimpl.
		 */
		class MetalGpuProgram : public GpuProgram
		{
		public:
			MetalGpuProgram(MetalGpuDevice& gpuDevice, const GpuProgramCreateInformation& createInformation);
			~MetalGpuProgram() override;

			void Initialize() override;

			/** Returns the compute workgroup size recorded by the high-level shader compiler. */
			const u32* GetWorkgroupSize() const { return mWorkgroupSize; }

#ifdef __OBJC__
			/** Returns the underlying MTLFunction. May be nil if Initialize() failed or has not been called yet. */
			id<MTLFunction> GetMetalFunction() const;

			/** Returns the underlying MTLLibrary that owns the function. */
			id<MTLLibrary> GetMetalLibrary() const;
#endif

		private:
			struct Impl;

			MetalGpuDevice& mGpuDevice;
			TUnique<Impl> mImpl;
			u32 mWorkgroupSize[3] = { 1, 1, 1 };
		};

		/** @} */
	} // namespace render
} // namespace b3d
