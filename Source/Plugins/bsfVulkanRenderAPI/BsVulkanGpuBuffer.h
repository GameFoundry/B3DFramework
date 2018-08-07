//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	DirectX 11 implementation of a generic GPU buffer. */
	class VulkanGpuBuffer : public GpuBuffer
	{
	public:
		~VulkanGpuBuffer();

		/** @copydoc GpuBuffer::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::unlock */
		void unlock() override;

		/** @copydoc GpuBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, 
			UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
		
		/** 
		 * Gets the resource wrapping the buffer object, on the specified device. If the object wasn't initialized for the
		 * specified device, null is returned. 
		 */
		VulkanBuffer* getResource(UINT32 deviceIdx) const;

		/** 
		 * Returns a view of the buffer object using the format specified when creating this object, per device. If the
		 * object wasn't initialized for the specified device, null is returned.
		 */
		VkBufferView getView(UINT32 deviceIdx) const;
	protected:
		friend class VulkanHardwareBufferManager;

		VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, const SPtr<VulkanHardwareBuffer>& underlyingBuffer);

		/** @copydoc GpuBuffer::initialize */
		void initialize() override;

		/** Updates buffer views if they are missing or don't match the buffers. */
		void updateViews();
	private:
		VulkanHardwareBuffer* mBuffer = nullptr;
		VkBuffer mCachedBuffers[BS_MAX_DEVICES] { };
		VkBufferView mBufferViews[BS_MAX_DEVICES] { };

		VkBufferViewCreateInfo mViewCI;
	};

	/** @} */
}}