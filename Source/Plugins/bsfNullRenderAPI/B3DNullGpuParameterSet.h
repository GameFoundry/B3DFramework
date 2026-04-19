//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuParameterSet.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/** Null implementation of GpuParameterSet, containing resource descriptors for a single descriptor set. */
		class NullGpuParameters : public GpuParameterSet
		{
		public:
			NullGpuParameters(NullGpuDevice& gpuDevice, const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout);
			~NullGpuParameters() override = default;

			bool SetUniformBuffer(u32 slot, const SPtr<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override { return true; }
			bool SetSampledTexture(u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override { return true; }
			bool SetStorageTexture(u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override { return true; }
			bool SetStorageBuffer(u32 slot, const SPtr<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override { return true; }
			bool SetSamplerState(u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex = 0) override { return true; }

			void Initialize() override {}
		};

		/** @} */
	} // namespace render
} // namespace b3d
