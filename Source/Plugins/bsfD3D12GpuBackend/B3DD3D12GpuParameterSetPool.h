//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"

namespace b3d::render
{
	class D3D12GpuDevice;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/**
	 * DirectX 12 implementation of a GPU parameter set pool. Parameter sets allocate their GPU-visible descriptor
	 * ranges from the device's descriptor manager on first bind, so the pool itself only enforces the set capacity.
	 */
	class D3D12GpuParameterSetPool : public GpuParameterSetPool
	{
	public:
		D3D12GpuParameterSetPool(D3D12GpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation);

		TShared<GpuParameterSet> Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize = false) override;
		void Reset() override;

	private:
		D3D12GpuDevice& mDevice;
		u32 mAllocatedSetCount = 0;
	};

	/** @} */
} // namespace b3d::render
