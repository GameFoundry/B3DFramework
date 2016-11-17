//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanHardwareBufferManager.h"
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanGpuParams.h"
#include "BsGpuParamDesc.h"

namespace BansheeEngine
{
	VulkanHardwareBufferCoreManager::VulkanHardwareBufferCoreManager()
	{ }

	SPtr<VertexBufferCore> VulkanHardwareBufferCoreManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanVertexBufferCore> ret = bs_shared_ptr_new<VulkanVertexBufferCore>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> VulkanHardwareBufferCoreManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VulkanIndexBufferCore> ret = bs_shared_ptr_new<VulkanIndexBufferCore>( desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBufferCore> VulkanHardwareBufferCoreManager::createGpuParamBlockBufferInternal(UINT32 size,
		GpuParamBlockUsage usage, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParamBlockBufferCore* paramBlockBuffer =
			new (bs_alloc<VulkanGpuParamBlockBufferCore>()) VulkanGpuParamBlockBufferCore(size, usage, deviceMask);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<VulkanGpuParamBlockBufferCore>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> VulkanHardwareBufferCoreManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		VulkanGpuBufferCore* buffer = new (bs_alloc<VulkanGpuBufferCore>()) VulkanGpuBufferCore(desc, deviceMask);

		SPtr<VulkanGpuBufferCore> bufferPtr = bs_shared_ptr<VulkanGpuBufferCore>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

	SPtr<GpuParamsCore> VulkanHardwareBufferCoreManager::createGpuParamsInternal(
		const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
	{
		VulkanGpuParams* params = new (bs_alloc<VulkanGpuParams>()) VulkanGpuParams(paramInfo, deviceMask);
		SPtr<GpuParamsCore> paramsPtr = bs_shared_ptr<GpuParamsCore>(params);
		paramsPtr->_setThisPtr(paramsPtr);

		return paramsPtr;
	}
}