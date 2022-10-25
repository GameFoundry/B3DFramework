//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsHardwareBufferManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	Handles creation of Vulkan hardware buffers. */
		class VulkanHardwareBufferManager : public HardwareBufferManager
		{
		public:
			VulkanHardwareBufferManager();
			~VulkanHardwareBufferManager();

			/** Returns a buffer that can be used for buffer read operations when no other buffer is bound. */
			VulkanHardwareBuffer* GetDummyReadBuffer() const { return mDummyReadBuffer; }

			/** Returns a buffer that can be used for buffer storage operations when no other buffer is bound. */
			VulkanHardwareBuffer* GetDummyStorageBuffer() const { return mDummyStorageBuffer; }

			/** Returns a buffer that can be used for uniform storage when no other buffer is bound. */
			VulkanHardwareBuffer* GetDummyUniformBuffer() const { return mDummyUniformBuffer; }

			/** Returns a buffer that can be used for structured storage when no other buffer is bound. */
			VulkanHardwareBuffer* GetDummyStructuredBuffer() const { return mDummyStructuredBuffer; }

		protected:
			/** @copydoc HardwareBufferManager::createVertexBufferInternal */
			SPtr<VertexBuffer> CreateVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createIndexBufferInternal */
			SPtr<IndexBuffer> CreateIndexBufferInternal(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createGpuParamBlockBufferInternal  */
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC&, GpuDeviceFlags) */
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC&, SPtr<HardwareBuffer>) */
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer);

			/** @copydoc HardwareBufferManager::createGpuParamsInternal */
			SPtr<GpuParams> CreateGpuParamsInternal(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			VulkanHardwareBuffer* mDummyReadBuffer;
			VulkanHardwareBuffer* mDummyStorageBuffer;
			VulkanHardwareBuffer* mDummyUniformBuffer;
			VulkanHardwareBuffer* mDummyStructuredBuffer;
		};

		/** @} */
	} // namespace ct
} // namespace bs
