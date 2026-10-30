//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DSamplerState.h"
#include "RTTI/B3DSamplerStateRTTI.h"
#include "GpuBackend/B3DGpuBindlessHeap.h"
#include "GpuBackend/B3DGpuDevice.h"

using namespace b3d;

bool SamplerStateInformation::operator==(const SamplerStateInformation& rhs) const
{
	return AddressMode == rhs.AddressMode &&
		MinFilter == rhs.MinFilter &&
		MagFilter == rhs.MagFilter &&
		MipFilter == rhs.MipFilter &&
		MaxAniso == rhs.MaxAniso &&
		MipmapBias == rhs.MipmapBias &&
		MipMin == rhs.MipMin &&
		MipMax == rhs.MipMax &&
		BorderColor == rhs.BorderColor &&
		ComparisonFunc == rhs.ComparisonFunc;
}

SamplerState::SamplerState(GpuDevice& device, const SamplerStateCreateInformation& createInformation)
	: mInformation(createInformation), mDevice(device)
{
}

SamplerState::~SamplerState()
{
	const u32 bindlessIndex = mBindlessIndex.load(std::memory_order_acquire);
	if(bindlessIndex == kGpuBindlessInvalidIndex)
		return;

	mBindlessHeaps->ReleaseSlot(GpuBindlessHeapType::Sampler, bindlessIndex, this);

	if(mGpuResource != nullptr)
		mBindlessHeaps->UnpinResource(*mGpuResource);
}

u32 SamplerState::GetBindlessIndex() const
{
	const u32 existingIndex = mBindlessIndex.load(std::memory_order_acquire);
	if(existingIndex != kGpuBindlessInvalidIndex)
		return existingIndex;

	GpuBindlessHeaps* heaps = mDevice.GetBindlessHeaps();
	if(heaps == nullptr)
		return kGpuBindlessInvalidIndex;

	const u32 newIndex = heaps->AllocateSamplerSlot(*this, this);
	if(newIndex == kGpuBindlessInvalidIndex)
		return kGpuBindlessInvalidIndex;

	// Another thread may have allocated a slot concurrently, in which case its slot is used
	u32 expectedIndex = kGpuBindlessInvalidIndex;
	if(!mBindlessIndex.compare_exchange_strong(expectedIndex, newIndex, std::memory_order_acq_rel))
	{
		heaps->ReleaseSlot(GpuBindlessHeapType::Sampler, newIndex, this);
		return expectedIndex;
	}

	// The slot is released into these heaps even if the device no longer reports them, as when the heaps destroy the global samplers
	mBindlessHeaps = heaps;
	if(mGpuResource != nullptr)
		heaps->PinResource(*mGpuResource);

	return newIndex;
}

u64 SamplerState::GenerateHash(const SamplerStateInformation& information)
{
	size_t hash = 0;
	B3DCombineHash(hash, (u32)information.AddressMode.U);
	B3DCombineHash(hash, (u32)information.AddressMode.V);
	B3DCombineHash(hash, (u32)information.AddressMode.W);
	B3DCombineHash(hash, (u32)information.MinFilter);
	B3DCombineHash(hash, (u32)information.MagFilter);
	B3DCombineHash(hash, (u32)information.MipFilter);
	B3DCombineHash(hash, information.MaxAniso);
	B3DCombineHash(hash, information.MipmapBias);
	B3DCombineHash(hash, information.MipMin);
	B3DCombineHash(hash, information.MipMax);
	B3DCombineHash(hash, information.BorderColor);
	B3DCombineHash(hash, (u32)information.ComparisonFunc);

	return (u64)hash;
}

/************************************************************************/
/* 								RTTI		                     		*/
/************************************************************************/

RTTIType* SamplerState::GetRttiStatic()
{
	return SamplerStateRTTI::Instance();
}

RTTIType* SamplerState::GetRtti() const
{
	return SamplerState::GetRttiStatic();
}
