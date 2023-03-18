//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanHardwareBufferManager.h"
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanGenericGpuBuffer.h"
#include "BsVulkanGpuBackend.h"
#include "BsVulkanGpuParamBlockBuffer.h"
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

SPtr<ct::VertexBuffer> VulkanHardwareBufferManager::CreateVertexBufferInternal(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<VulkanVertexBuffer> ret = B3DMakeShared<VulkanVertexBuffer>(desc, deviceMask);
	ret->SetShared(ret);

	return ret;
}

SPtr<ct::IndexBuffer> VulkanHardwareBufferManager::CreateIndexBufferInternal(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<VulkanIndexBuffer> ret = B3DMakeShared<VulkanIndexBuffer>(desc, deviceMask);
	ret->SetShared(ret);

	return ret;
}

SPtr<ct::GpuParamBlockBuffer> VulkanHardwareBufferManager::CreateGpuParamBlockBufferInternal(u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask)
{
	VulkanGpuParamBlockBuffer* paramBlockBuffer =
		new(B3DAllocate<VulkanGpuParamBlockBuffer>()) VulkanGpuParamBlockBuffer(size, flags, deviceMask);

	SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = B3DMakeSharedFromExisting<VulkanGpuParamBlockBuffer>(paramBlockBuffer);
	paramBlockBufferPtr->SetShared(paramBlockBufferPtr);

	return paramBlockBufferPtr;
}

SPtr<ct::GpuParamBlockBuffer> VulkanHardwareBufferManager::CreateGpuParamBlockBufferInternal(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size)
{
	VulkanGpuParamBlockBuffer* parameterBlockBuffer =
		new(B3DAllocate<VulkanGpuParamBlockBuffer>()) VulkanGpuParamBlockBuffer(backingMemory, offset, size);

	SPtr<GpuParamBlockBuffer> parameterBlockBufferShared = B3DMakeSharedFromExisting<VulkanGpuParamBlockBuffer>(parameterBlockBuffer);
	parameterBlockBufferShared->SetShared(parameterBlockBufferShared);

	return parameterBlockBufferShared;
}

SPtr<ct::GenericGpuBuffer> VulkanHardwareBufferManager::CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	VulkanGenericGpuBuffer* buffer = new(B3DAllocate<VulkanGenericGpuBuffer>()) VulkanGenericGpuBuffer(desc, deviceMask);

	SPtr<VulkanGenericGpuBuffer> bufferPtr = B3DMakeSharedFromExisting<VulkanGenericGpuBuffer>(buffer);
	bufferPtr->SetShared(bufferPtr);

	return bufferPtr;
}

SPtr<ct::GenericGpuBuffer> VulkanHardwareBufferManager::CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer)
{
	VulkanGenericGpuBuffer* buffer = new(B3DAllocate<VulkanGenericGpuBuffer>()) VulkanGenericGpuBuffer(desc, std::move(underlyingBuffer));

	SPtr<VulkanGenericGpuBuffer> bufferPtr = B3DMakeSharedFromExisting<VulkanGenericGpuBuffer>(buffer);
	bufferPtr->SetShared(bufferPtr);

	return bufferPtr;
}

SPtr<ct::GpuParams> VulkanHardwareBufferManager::CreateGpuParamsInternal(
	const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	VulkanGpuParams* params = new(B3DAllocate<VulkanGpuParams>()) VulkanGpuParams(paramInfo, deviceMask);
	SPtr<GpuParams> paramsPtr = B3DMakeSharedFromExisting<GpuParams>(params);
	paramsPtr->SetShared(paramsPtr);

	return paramsPtr;
}
