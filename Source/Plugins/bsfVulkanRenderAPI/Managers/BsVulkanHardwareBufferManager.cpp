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

		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			for(UINT32 j = 0; j < BF_COUNT; j++)
			{
				mDummyReadBufferViews[i][j] = VK_NULL_HANDLE;
				mDummyStorageBufferViews[i][j] = VK_NULL_HANDLE;
			}
		}
	}

	VulkanHardwareBufferManager::~VulkanHardwareBufferManager()
	{
		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			VulkanBuffer* readBuffer = mDummyReadBuffer->getResource(i);
			if (readBuffer)
			{
				for (UINT32 j = 0; j < BF_COUNT; j++)
				{
					if(mDummyReadBufferViews[i][j] != VK_NULL_HANDLE)
						vkDestroyBufferView(readBuffer->getDevice().getLogical(), mDummyReadBufferViews[i][j], gVulkanAllocator);
				}
			}

			VulkanBuffer* storageBuffer = mDummyStorageBuffer->getResource(i);
			if (storageBuffer)
			{
				for (UINT32 j = 0; j < BF_COUNT; j++)
				{
					if(mDummyStorageBufferViews[i][j] != VK_NULL_HANDLE)
						vkDestroyBufferView(storageBuffer->getDevice().getLogical(), mDummyStorageBufferViews[i][j], gVulkanAllocator);
				}
			}
		}

		bs_delete(mDummyReadBuffer);
		bs_delete(mDummyStorageBuffer);
		bs_delete(mDummyUniformBuffer);
		bs_delete(mDummyStructuredBuffer);
	}

	VkBufferView VulkanHardwareBufferManager::getDummyReadBufferView(GpuBufferFormat elementType, UINT32 deviceIdx) const
	{
		UINT32 elementIdx = (UINT32)elementType;
		if(mDummyReadBufferViews[deviceIdx][elementIdx] == VK_NULL_HANDLE)
		{
			VulkanBuffer* readBuffer = mDummyReadBuffer->getResource(deviceIdx);
			if (readBuffer)
			{
				VkBufferViewCreateInfo viewCI;
				viewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				viewCI.pNext = nullptr;
				viewCI.flags = 0;
				viewCI.format = VulkanUtility::getBufferFormat(elementType);
				viewCI.offset = 0;
				viewCI.range = VK_WHOLE_SIZE;
				viewCI.buffer = readBuffer->getHandle();

				VkResult result = vkCreateBufferView(readBuffer->getDevice().getLogical(), &viewCI, gVulkanAllocator,
					&mDummyReadBufferViews[deviceIdx][elementIdx]);
				assert(result == VK_SUCCESS);
			}
		}

		return mDummyReadBufferViews[deviceIdx][elementIdx];
	}

	VkBufferView VulkanHardwareBufferManager::getDummyStorageBufferView(GpuBufferFormat elementType, UINT32 deviceIdx) const
	{
		UINT32 elementIdx = (UINT32)elementType;
		if(mDummyStorageBufferViews[deviceIdx][elementIdx] == VK_NULL_HANDLE)
		{
			VulkanBuffer* readBuffer = mDummyStorageBuffer->getResource(deviceIdx);
			if (readBuffer)
			{
				VkBufferViewCreateInfo viewCI;
				viewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				viewCI.pNext = nullptr;
				viewCI.flags = 0;
				viewCI.format = VulkanUtility::getBufferFormat(elementType);
				viewCI.offset = 0;
				viewCI.range = VK_WHOLE_SIZE;
				viewCI.buffer = readBuffer->getHandle();

				VkResult result = vkCreateBufferView(readBuffer->getDevice().getLogical(), &viewCI, gVulkanAllocator,
					&mDummyStorageBufferViews[deviceIdx][elementIdx]);
				assert(result == VK_SUCCESS);
			}
		}

		return mDummyStorageBufferViews[deviceIdx][elementIdx];
	}

	VkBuffer VulkanHardwareBufferManager::getDummyUniformBuffer(UINT32 deviceIdx) const
	{
		return mDummyUniformBuffer->getResource(deviceIdx)->getHandle();
	}

	VkBuffer VulkanHardwareBufferManager::getDummyStructuredBuffer(UINT32 deviceIdx) const
	{
		return mDummyStructuredBuffer->getResource(deviceIdx)->getHandle();
	}

	SPtr<VertexBuffer> VulkanHardwareBufferManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanVertexBuffer> ret = bs_shared_ptr_new<VulkanVertexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBuffer> VulkanHardwareBufferManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanIndexBuffer> ret = bs_shared_ptr_new<VulkanIndexBuffer>( desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBuffer> VulkanHardwareBufferManager::createGpuParamBlockBufferInternal(UINT32 size,
		GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParamBlockBuffer* paramBlockBuffer =
			new (bs_alloc<VulkanGpuParamBlockBuffer>()) VulkanGpuParamBlockBuffer(size, usage, deviceMask);

		SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = bs_shared_ptr<VulkanGpuParamBlockBuffer>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBuffer> VulkanHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		VulkanGpuBuffer* buffer = new (bs_alloc<VulkanGpuBuffer>()) VulkanGpuBuffer(desc, deviceMask);

		SPtr<VulkanGpuBuffer> bufferPtr = bs_shared_ptr<VulkanGpuBuffer>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

	SPtr<GpuBuffer> VulkanHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		SPtr<HardwareBuffer> underlyingBuffer)
	{
		VulkanGpuBuffer* buffer = new (bs_alloc<VulkanGpuBuffer>()) VulkanGpuBuffer(desc, std::move(underlyingBuffer));

		SPtr<VulkanGpuBuffer> bufferPtr = bs_shared_ptr<VulkanGpuBuffer>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

	SPtr<GpuParams> VulkanHardwareBufferManager::createGpuParamsInternal(
		const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParams* params = new (bs_alloc<VulkanGpuParams>()) VulkanGpuParams(paramInfo, deviceMask);
		SPtr<GpuParams> paramsPtr = bs_shared_ptr<GpuParams>(params);
		paramsPtr->_setThisPtr(paramsPtr);

		return paramsPtr;
	}
}}
