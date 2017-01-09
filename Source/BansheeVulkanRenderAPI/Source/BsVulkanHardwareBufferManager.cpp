//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanHardwareBufferManager.h"
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanGpuParams.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsGpuParamDesc.h"

namespace bs { namespace ct
{
	VulkanHardwareBufferManager::VulkanHardwareBufferManager()
	{
		// Note: When multi-GPU is properly tested, make sure to create these textures on all GPUs
		mDummyReadBuffer = bs_new<VulkanHardwareBuffer>(
			VulkanHardwareBuffer::BT_GENERIC, BF_32X1F, GBU_STATIC, 16, GDF_DEFAULT);

		mDummyStorageBuffer = bs_new<VulkanHardwareBuffer>(
			VulkanHardwareBuffer::BT_STORAGE, BF_32X1F, GBU_STATIC, 16, GDF_DEFAULT);

		mDummyUniformBuffer = bs_new<VulkanHardwareBuffer>(
			VulkanHardwareBuffer::BT_UNIFORM, BF_UNKNOWN, GBU_STATIC, 16, GDF_DEFAULT);
	}

	VulkanHardwareBufferManager::~VulkanHardwareBufferManager()
	{
		bs_delete(mDummyReadBuffer);
		bs_delete(mDummyStorageBuffer);
		bs_delete(mDummyUniformBuffer);
	}

	VkBufferView VulkanHardwareBufferManager::getDummyReadBufferView(UINT32 deviceIdx) const
	{
		return mDummyReadBuffer->getResource(deviceIdx)->getView();
	}

	VkBufferView VulkanHardwareBufferManager::getDummyStorageBufferView(UINT32 deviceIdx) const
	{
		return mDummyStorageBuffer->getResource(deviceIdx)->getView();
	}

	VkBuffer VulkanHardwareBufferManager::getDummyUniformBuffer(UINT32 deviceIdx) const
	{
		return mDummyUniformBuffer->getResource(deviceIdx)->getHandle();
	}

	SPtr<VertexBufferCore> VulkanHardwareBufferManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanVertexBuffer> ret = bs_shared_ptr_new<VulkanVertexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> VulkanHardwareBufferManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanIndexBuffer> ret = bs_shared_ptr_new<VulkanIndexBuffer>( desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBufferCore> VulkanHardwareBufferManager::createGpuParamBlockBufferInternal(UINT32 size,
		GpuParamBlockUsage usage, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParamBlockBuffer* paramBlockBuffer =
			new (bs_alloc<VulkanGpuParamBlockBuffer>()) VulkanGpuParamBlockBuffer(size, usage, deviceMask);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<VulkanGpuParamBlockBuffer>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> VulkanHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		VulkanGpuBuffer* buffer = new (bs_alloc<VulkanGpuBuffer>()) VulkanGpuBuffer(desc, deviceMask);

		SPtr<VulkanGpuBuffer> bufferPtr = bs_shared_ptr<VulkanGpuBuffer>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

	SPtr<GpuParamsCore> VulkanHardwareBufferManager::createGpuParamsInternal(
		const SPtr<GpuPipelineParamInfoCore>& paramInfo, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParams* params = new (bs_alloc<VulkanGpuParams>()) VulkanGpuParams(paramInfo, deviceMask);
		SPtr<GpuParamsCore> paramsPtr = bs_shared_ptr<GpuParamsCore>(params);
		paramsPtr->_setThisPtr(paramsPtr);

		return paramsPtr;
	}
}}