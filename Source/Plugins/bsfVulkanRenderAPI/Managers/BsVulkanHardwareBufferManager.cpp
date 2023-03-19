//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanHardwareBufferManager.h"
#include "BsVulkanGpuBackend.h"
#include "BsVulkanGpuParams.h"
#include "BsVulkanGpuBuffer.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "BsVulkanUtility.h"
#include "BsVulkanGpuDevice.h"

using namespace bs;
using namespace bs::ct;

VulkanHardwareBufferManager::VulkanHardwareBufferManager()
{
	const SPtr<VulkanGpuDevice>& vulkanGpuDevice = GetVulkanGpuBackend().GetVulkanDevice(0);
	if(!B3D_ENSURE(vulkanGpuDevice != nullptr))
		return;

	// Note: When multi-GPU is properly tested, make sure to create these textures on all GPUs
	mDummyReadBuffer = std::static_pointer_cast<VulkanGpuBuffer>(vulkanGpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, 1, GpuBufferFlag::StoreOnGPU)));
	mDummyStorageBuffer = std::static_pointer_cast<VulkanGpuBuffer>(vulkanGpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, 1, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowWritesOnTheGPU)));
	mDummyUniformBuffer = std::static_pointer_cast<VulkanGpuBuffer>(vulkanGpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(16, GpuBufferFlag::StoreOnGPU)));
	mDummyStructuredBuffer = std::static_pointer_cast<VulkanGpuBuffer>(vulkanGpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(16, 1, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowWritesOnTheGPU)));
	mDummyVertexBuffer = std::static_pointer_cast<VulkanGpuBuffer>(vulkanGpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(16, 1, GpuBufferFlag::StoreOnGPU)));
}

SPtr<ct::GpuParams> VulkanHardwareBufferManager::CreateGpuParamsInternal(
	const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	VulkanGpuParams* params = new(B3DAllocate<VulkanGpuParams>()) VulkanGpuParams(paramInfo, deviceMask);
	SPtr<GpuParams> paramsPtr = B3DMakeSharedFromExisting<GpuParams>(params);
	paramsPtr->SetShared(paramsPtr);

	return paramsPtr;
}
