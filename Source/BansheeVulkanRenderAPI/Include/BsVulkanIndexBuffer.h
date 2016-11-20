//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsIndexBuffer.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Vulkan implementation of an index buffer. */
	class VulkanIndexBufferCore : public IndexBufferCore
	{
	public:
		VulkanIndexBufferCore(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~VulkanIndexBufferCore();

		/** @copydoc IndexBufferCore::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc IndexBufferCore::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc IndexBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, UINT32 queueIdx = 0) override;

		/** 
		 * Gets the resource wrapping the buffer object, on the specified device. If GPU param block buffer's device mask
		 * doesn't include the provided device, null is returned. 
		 */
		VulkanBuffer* getResource(UINT32 deviceIdx) const;

	protected:
		/** @copydoc IndexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc IndexBufferCore::unmap */
		void unmap() override;

		/** @copydoc IndexBufferCore::initialize */
		void initialize() override;

	private:
		VulkanHardwareBuffer* mBuffer;
		GpuBufferUsage mUsage;
		GpuDeviceFlags mDeviceMask;
	};

	/** @} */
}