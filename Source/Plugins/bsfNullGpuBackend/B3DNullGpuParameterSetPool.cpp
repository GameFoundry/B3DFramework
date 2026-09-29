//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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

	TShared<GpuParameterSet> NullGpuParameterSetPool::Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, GpuObjectCreateFlags flags)
	{
		if (mAllocatedSetCount >= mInformation.MaxSets)
			return nullptr;

		NullGpuParameters* rawParamSet = new(B3DAllocate<NullGpuParameters>()) NullGpuParameters(mDevice, layout);

		TShared<NullGpuParameters> paramSet = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
			? B3DMakeSharedFromExisting(rawParamSet)
			: GpuDevice::MakeSharedStandalone(rawParamSet);

		if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
			paramSet->Initialize();

		mAllocatedSetCount++;
		return paramSet;
	}

	void NullGpuParameterSetPool::Reset()
	{
		mAllocatedSetCount = 0;
	}
} // namespace b3d::render
