//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuParameterSetPool.h"
#include "B3DNullGpuDevice.h"
#include "B3DNullGpuParameterSet.h"

namespace b3d::render
{
	NullGpuParameterSetPool::NullGpuParameterSetPool(NullGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation)
		: GpuParameterSetPool(createInformation)
		, mDevice(device)
	{
	}

	SPtr<GpuParameterSet> NullGpuParameterSetPool::Create(const SPtr<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize)
	{
		if (mAllocatedSetCount >= mInformation.MaxSets)
			return nullptr;

		auto paramSet = B3DMakeShared<NullGpuParameters>(mDevice, layout);

		if (!deferredInitialize)
			paramSet->Initialize();

		mAllocatedSetCount++;
		return paramSet;
	}

	void NullGpuParameterSetPool::Reset()
	{
		mAllocatedSetCount = 0;
	}
} // namespace b3d::render
