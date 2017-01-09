//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsHardwareBufferManager.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Handles creation of Vulkan hardware buffers. */
	class VulkanHardwareBufferManager : public HardwareBufferCoreManager
	{
	public:
		VulkanHardwareBufferManager();
		~VulkanHardwareBufferManager();

		/** Returns a buffer view that can be used for buffer read operations when no other buffer is bound. */
		VkBufferView getDummyReadBufferView(UINT32 deviceIdx) const;

		/** Returns a buffer view that can be used for buffer storage operations when no other buffer is bound. */
		VkBufferView getDummyStorageBufferView(UINT32 deviceIdx) const;

		/** Returns a buffer that can be used for uniform storage when no other buffer is bound. */
		VkBuffer getDummyUniformBuffer(UINT32 deviceIdx) const;

	protected:     
		/** @copydoc HardwareBufferCoreManager::createVertexBufferInternal */
		SPtr<VertexBufferCore> createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferCoreManager::createIndexBufferInternal */
		SPtr<IndexBufferCore> createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferCoreManager::createGpuParamBlockBufferInternal  */
		SPtr<GpuParamBlockBufferCore> createGpuParamBlockBufferInternal(UINT32 size, 
			GpuParamBlockUsage usage = GPBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferCoreManager::createGpuBufferInternal */
		SPtr<GpuBufferCore> createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferCoreManager::createGpuParamsInternal */
		SPtr<GpuParamsCore> createGpuParamsInternal(const SPtr<GpuPipelineParamInfoCore>& paramInfo,
													GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		VulkanHardwareBuffer* mDummyReadBuffer;
		VulkanHardwareBuffer* mDummyStorageBuffer;
		VulkanHardwareBuffer* mDummyUniformBuffer;
	};

	/** @} */
}}