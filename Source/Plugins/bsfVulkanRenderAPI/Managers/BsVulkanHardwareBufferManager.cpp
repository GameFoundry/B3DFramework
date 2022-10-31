//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanHardwareBufferManager.h"
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanGpuParams.h"
#include "BsVulkanHardwareBuffer.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"

using namespace bs;
using namespace bs::ct;

VulkanHardwareBufferManager::VulkanHardwareBufferManager()
{
	// Note: When multi-GPU is properly tested, make sure to create these textures on all GPUs
	mDummyReadBuffer = B3DNew<VulkanHardwareBuffer>(
		VulkanHardwareBuffer::BT_GENERIC, BF_32X1F, GBU_STATIC, 16, GDF_DEFAULT);

	mDummyStorageBuffer = B3DNew<VulkanHardwareBuffer>(
		VulkanHardwareBuffer::BT_GENERIC, BF_32X1F, GBU_LOADSTORE, 16, GDF_DEFAULT);

	mDummyUniformBuffer = B3DNew<VulkanHardwareBuffer>(
		VulkanHardwareBuffer::BT_UNIFORM, BF_UNKNOWN, GBU_STATIC, 16, GDF_DEFAULT);

	mDummyStructuredBuffer = B3DNew<VulkanHardwareBuffer>(
		VulkanHardwareBuffer::BT_STRUCTURED, BF_UNKNOWN, GBU_LOADSTORE, 16, GDF_DEFAULT);
}

VulkanHardwareBufferManager::~VulkanHardwareBufferManager()
{
	B3DDelete(mDummyReadBuffer);
	B3DDelete(mDummyStorageBuffer);
	B3DDelete(mDummyUniformBuffer);
	B3DDelete(mDummyStructuredBuffer);
}

SPtr<ct::VertexBuffer> VulkanHardwareBufferManager::CreateVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<VulkanVertexBuffer> ret = B3DMakeShared<VulkanVertexBuffer>(desc, deviceMask);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<ct::IndexBuffer> VulkanHardwareBufferManager::CreateIndexBufferInternal(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<VulkanIndexBuffer> ret = B3DMakeShared<VulkanIndexBuffer>(desc, deviceMask);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<ct::GpuParamBlockBuffer> VulkanHardwareBufferManager::CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
{
	VulkanGpuParamBlockBuffer* paramBlockBuffer =
		new(B3DAllocate<VulkanGpuParamBlockBuffer>()) VulkanGpuParamBlockBuffer(size, usage, deviceMask);

	SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = B3DMakeSharedFromExisting<VulkanGpuParamBlockBuffer>(paramBlockBuffer);
	paramBlockBufferPtr->SetThisPtrInternal(paramBlockBufferPtr);

	return paramBlockBufferPtr;
}

SPtr<ct::GpuBuffer> VulkanHardwareBufferManager::CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	VulkanGpuBuffer* buffer = new(B3DAllocate<VulkanGpuBuffer>()) VulkanGpuBuffer(desc, deviceMask);

	SPtr<VulkanGpuBuffer> bufferPtr = B3DMakeSharedFromExisting<VulkanGpuBuffer>(buffer);
	bufferPtr->SetThisPtrInternal(bufferPtr);

	return bufferPtr;
}

SPtr<ct::GpuBuffer> VulkanHardwareBufferManager::CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
{
	VulkanGpuBuffer* buffer = new(B3DAllocate<VulkanGpuBuffer>()) VulkanGpuBuffer(desc, std::move(underlyingBuffer));

	SPtr<VulkanGpuBuffer> bufferPtr = B3DMakeSharedFromExisting<VulkanGpuBuffer>(buffer);
	bufferPtr->SetThisPtrInternal(bufferPtr);

	return bufferPtr;
}

SPtr<ct::GpuParams> VulkanHardwareBufferManager::CreateGpuParamsInternal(
	const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	VulkanGpuParams* params = new(B3DAllocate<VulkanGpuParams>()) VulkanGpuParams(paramInfo, deviceMask);
	SPtr<GpuParams> paramsPtr = B3DMakeSharedFromExisting<GpuParams>(params);
	paramsPtr->SetThisPtrInternal(paramsPtr);

	return paramsPtr;
}
