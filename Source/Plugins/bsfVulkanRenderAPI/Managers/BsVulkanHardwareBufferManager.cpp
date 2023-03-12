//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanHardwareBufferManager.h"
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanGenericGpuBuffer.h"
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
	// Note: When multi-GPU is properly tested, make sure to create these textures on all GPUs
	mDummyReadBuffer = B3DNew<VulkanGpuBuffer>(
		GpuBufferType::Generic, GpuBufferFlag::StoreOnGPU, 16, GDF_DEFAULT);

	mDummyStorageBuffer = B3DNew<VulkanGpuBuffer>(
		GpuBufferType::Generic, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowWritesOnTheGPU, 16, GDF_DEFAULT);

	mDummyUniformBuffer = B3DNew<VulkanGpuBuffer>(
		GpuBufferType::Uniform, GpuBufferFlag::StoreOnGPU, 16, GDF_DEFAULT);

	mDummyStructuredBuffer = B3DNew<VulkanGpuBuffer>(
		GpuBufferType::Structured, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowWritesOnTheGPU, 16, GDF_DEFAULT);

	mDummyVertexBuffer = B3DNew<VulkanGpuBuffer>(
		GpuBufferType::Vertex, GpuBufferFlag::StoreOnGPU, 16, GDF_DEFAULT);
}

VulkanHardwareBufferManager::~VulkanHardwareBufferManager()
{
	B3DDelete(mDummyReadBuffer);
	B3DDelete(mDummyStorageBuffer);
	B3DDelete(mDummyUniformBuffer);
	B3DDelete(mDummyStructuredBuffer);
	B3DDelete(mDummyVertexBuffer);
}

SPtr<GpuBuffer> VulkanHardwareBufferManager::CreateHardwareBuffer(GpuBufferType type, u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask)
{
	return B3DMakeShared<VulkanGpuBuffer>(type, flags, size, deviceMask);
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
