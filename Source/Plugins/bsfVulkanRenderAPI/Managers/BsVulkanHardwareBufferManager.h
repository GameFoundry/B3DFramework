//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsGpuBuffer.h"

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

			/** Returns a buffer that can be used for buffer read operations when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyReadBuffer() const { return mDummyReadBuffer.get(); }

			/** Returns a buffer that can be used for buffer storage operations when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyStorageBuffer() const { return mDummyStorageBuffer.get(); }

			/** Returns a buffer that can be used for uniform storage when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyUniformBuffer() const { return mDummyUniformBuffer.get(); }

			/** Returns a buffer that can be used for structured storage when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyStructuredBuffer() const { return mDummyStructuredBuffer.get(); }

			/** Returns a buffer that can be used for vertex buffers when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyVertexBuffer() const { return mDummyVertexBuffer.get(); }
		protected:
			SPtr<GpuParams> CreateGpuParamsInternal(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

			SPtr<VulkanGpuBuffer> mDummyReadBuffer;
			SPtr<VulkanGpuBuffer> mDummyStorageBuffer;
			SPtr<VulkanGpuBuffer> mDummyUniformBuffer;
			SPtr<VulkanGpuBuffer> mDummyStructuredBuffer;
			SPtr<VulkanGpuBuffer> mDummyVertexBuffer;
		};

		/** @} */
	} // namespace ct
} // namespace bs
