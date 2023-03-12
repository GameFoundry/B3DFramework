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

			SPtr<GpuBuffer> CreateHardwareBuffer(GpuBufferType type, u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask) override;

			/** Returns a buffer that can be used for buffer read operations when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyReadBuffer() const { return mDummyReadBuffer; }

			/** Returns a buffer that can be used for buffer storage operations when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyStorageBuffer() const { return mDummyStorageBuffer; }

			/** Returns a buffer that can be used for uniform storage when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyUniformBuffer() const { return mDummyUniformBuffer; }

			/** Returns a buffer that can be used for structured storage when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyStructuredBuffer() const { return mDummyStructuredBuffer; }

			/** Returns a buffer that can be used for vertex buffers when no other buffer is bound. */
			VulkanGpuBuffer* GetDummyVertexBuffer() const { return mDummyVertexBuffer; }
		protected:
			SPtr<VertexBuffer> CreateVertexBufferInternal(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<IndexBuffer> CreateIndexBufferInternal(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size) override;
			SPtr<GenericGpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GenericGpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer) override;
			SPtr<GpuParams> CreateGpuParamsInternal(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

			VulkanGpuBuffer* mDummyReadBuffer;
			VulkanGpuBuffer* mDummyStorageBuffer;
			VulkanGpuBuffer* mDummyUniformBuffer;
			VulkanGpuBuffer* mDummyStructuredBuffer;
			VulkanGpuBuffer* mDummyVertexBuffer;
		};

		/** @} */
	} // namespace ct
} // namespace bs
