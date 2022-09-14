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

namespace bs { namespace ct
{
	VulkanHardwareBufferManager::VulkanHardwareBufferManager()
	{
		// Note: When multi-GPU is properly tested, make sure to create these textures on all GPUs
		mDummyReadBuffer = bs_new<VulkanHardwareBuffer>(
			VulkanHardwareBuffer::BT_GENERIC, BF_32X1F, GBU_STATIC, 16, GDF_DEFAULT);

		mDummyStorageBuffer = bs_new<VulkanHardwareBuffer>(
			VulkanHardwareBuffer::BT_GENERIC, BF_32X1F, GBU_LOADSTORE, 16, GDF_DEFAULT);

		mDummyUniformBuffer = bs_new<VulkanHardwareBuffer>(
			VulkanHardwareBuffer::BT_UNIFORM, BF_UNKNOWN, GBU_STATIC, 16, GDF_DEFAULT);

		mDummyStructuredBuffer = bs_new<VulkanHardwareBuffer>(
			VulkanHardwareBuffer::BT_STRUCTURED, BF_UNKNOWN, GBU_LOADSTORE, 16, GDF_DEFAULT);
	}

	VulkanHardwareBufferManager::~VulkanHardwareBufferManager()
	{
		bs_delete(mDummyReadBuffer);
		bs_delete(mDummyStorageBuffer);
		bs_delete(mDummyUniformBuffer);
		bs_delete(mDummyStructuredBuffer);
	}

	SPtr<VertexBuffer> VulkanHardwareBufferManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanVertexBuffer> ret = bs_shared_ptr_new<VulkanVertexBuffer>(desc, deviceMask);
		ret->SetThisPtrInternal(ret);

		return ret;
	}

	SPtr<IndexBuffer> VulkanHardwareBufferManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanIndexBuffer> ret = bs_shared_ptr_new<VulkanIndexBuffer>( desc, deviceMask);
		ret->SetThisPtrInternal(ret);

		return ret;
	}

	SPtr<GpuParamBlockBuffer> VulkanHardwareBufferManager::createGpuParamBlockBufferInternal(UINT32 size,
		GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParamBlockBuffer* paramBlockBuffer =
			new (bs_alloc<VulkanGpuParamBlockBuffer>()) VulkanGpuParamBlockBuffer(size, usage, deviceMask);

		SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = bs_shared_ptr<VulkanGpuParamBlockBuffer>(paramBlockBuffer);
		paramBlockBufferPtr->SetThisPtrInternal(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBuffer> VulkanHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		VulkanGpuBuffer* buffer = new (bs_alloc<VulkanGpuBuffer>()) VulkanGpuBuffer(desc, deviceMask);

		SPtr<VulkanGpuBuffer> bufferPtr = bs_shared_ptr<VulkanGpuBuffer>(buffer);
		bufferPtr->SetThisPtrInternal(bufferPtr);

		return bufferPtr;
	}

	SPtr<GpuBuffer> VulkanHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		SPtr<HardwareBuffer> underlyingBuffer)
	{
		VulkanGpuBuffer* buffer = new (bs_alloc<VulkanGpuBuffer>()) VulkanGpuBuffer(desc, std::move(underlyingBuffer));

		SPtr<VulkanGpuBuffer> bufferPtr = bs_shared_ptr<VulkanGpuBuffer>(buffer);
		bufferPtr->SetThisPtrInternal(bufferPtr);

		return bufferPtr;
	}

	SPtr<GpuParams> VulkanHardwareBufferManager::createGpuParamsInternal(
		const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParams* params = new (bs_alloc<VulkanGpuParams>()) VulkanGpuParams(paramInfo, deviceMask);
		SPtr<GpuParams> paramsPtr = bs_shared_ptr<GpuParams>(params);
		paramsPtr->SetThisPtrInternal(paramsPtr);

		return paramsPtr;
	}
}}
