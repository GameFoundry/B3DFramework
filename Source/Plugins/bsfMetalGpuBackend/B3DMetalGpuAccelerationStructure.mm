//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuAccelerationStructure.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"

namespace b3d
{
	namespace render
	{
		MetalAccelerationStructure::MetalAccelerationStructure(MetalResourceManager* owner, id<MTLAccelerationStructure> accelerationStructure, const GpuAllocation& allocation,
			const StringView& name)
			: MetalResource(owner, name), mAccelerationStructure(accelerationStructure), mAllocation(allocation)
		{
			if (!name.empty())
			{
				const String nameCopy(name.data(), name.size());
				mAccelerationStructure.label = [NSString stringWithUTF8String:nameCopy.c_str()];
			}
		}

		MetalAccelerationStructure::~MetalAccelerationStructure()
		{
			GetDevice().GetHeapAllocator().Free(mAccelerationStructure, mAllocation);
			mAccelerationStructure = nil;
		}

		MetalGpuAccelerationStructure::MetalGpuAccelerationStructure(MetalGpuDevice& device, const GpuAccelerationStructureCreateInformation& createInformation,
			MetalAccelerationStructure& accelerationStructure)
			: GpuAccelerationStructure(device, createInformation, accelerationStructure), mAccelerationStructure(&accelerationStructure)
		{ }
	} // namespace render
} // namespace b3d
