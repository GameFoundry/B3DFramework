//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsHardwareBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Class containing common functionality for all Vulkan hardware buffers. */
	class VulkanHardwareBuffer : public HardwareBuffer
	{
		/** Information about allocated buffer memory for a single device. */
		struct MemoryInfo
		{
			SPtr<VulkanDevice> device;
			VkDeviceMemory memory;
		};

	public:
		VulkanHardwareBuffer(GpuBufferUsage usage, const VkMemoryRequirements& reqs, bool useSystemMem = false,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);
		~VulkanHardwareBuffer();

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 syncMask = 0x00000001) override;

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 syncMask = 0x00000001) override;

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, 
			UINT32 length, bool discardWholeBuffer = false, UINT32 syncMask = 0x00000001) override;

	protected:
		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		MemoryInfo mAllocations[BS_MAX_DEVICES];
	};

	/** @} */
}