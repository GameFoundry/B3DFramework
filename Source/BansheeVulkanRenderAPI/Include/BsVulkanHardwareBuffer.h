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
	public:
		VulkanHardwareBuffer(GpuBufferUsage usage, UINT32 elementCount, UINT32 elementSize, bool systemMemory = false, 
			bool streamOut = false, bool randomGpuWrite = false, bool useCounter = false);
		~VulkanHardwareBuffer();

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest) override;

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, 
			UINT32 length, bool discardWholeBuffer = false) override;

	protected:
		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		bool mRandomGpuWrite;
		bool mUseCounter;
		UINT32 mElementCount;
		UINT32 mElementSize;
	};

	/** @} */
}