//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuParameterSetPool.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuParameterSet.h"

namespace b3d::render
{
	D3D12GpuParameterSetPool::D3D12GpuParameterSetPool(D3D12GpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation)
		: GpuParameterSetPool(createInformation), mDevice(device)
	{
	}

	TShared<GpuParameterSet> D3D12GpuParameterSetPool::Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, GpuObjectCreateFlags flags)
	{
		if (mAllocatedSetCount >= mInformation.MaxSets)
			return nullptr;

		D3D12GpuParameters* rawParameterSet = new(B3DAllocate<D3D12GpuParameters>()) D3D12GpuParameters(layout, mDevice, setIndex);

		TShared<D3D12GpuParameters> parameterSet = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
			? B3DMakeSharedFromExisting(rawParameterSet)
			: GpuDevice::MakeSharedStandalone(rawParameterSet);

		parameterSet->SetShared(parameterSet);

		if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
			parameterSet->Initialize();

		mAllocatedSetCount++;
		return parameterSet;
	}

	void D3D12GpuParameterSetPool::Reset()
	{
		mAllocatedSetCount = 0;

		// TODO(d3d12-port): Transient sets keep their GPU-visible descriptor ranges until destruction; a pool-level
		// descriptor ring would allow reclaiming them in bulk here (See comments in descriptor manager code, using a linear allocator for transients sets would be good).
	}
} // namespace b3d::render
