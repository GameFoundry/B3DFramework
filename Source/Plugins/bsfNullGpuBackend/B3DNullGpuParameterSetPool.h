//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/** Null implementation of GpuParameterSetPool. */
		class NullGpuParameterSetPool final : public GpuParameterSetPool
		{
		public:
			NullGpuParameterSetPool(NullGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation);
			~NullGpuParameterSetPool() override = default;

			TShared<GpuParameterSet> Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize = false) override;
			void Reset() override;

		private:
			NullGpuDevice& mDevice;
			u32 mAllocatedSetCount = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
