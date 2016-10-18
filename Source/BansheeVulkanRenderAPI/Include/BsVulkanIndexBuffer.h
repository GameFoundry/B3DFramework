//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsIndexBuffer.h"
#include "BsVulkanHardwareBuffer.h"

namespace BansheeEngine
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
		void readData(UINT32 offset, UINT32 length, void* dest) override;

		/** @copydoc IndexBufferCore::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/** @copydoc IndexBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false) override;

	protected:
		/** @copydoc IndexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc IndexBufferCore::unmap */
		void unmap() override;

		/** @copydoc IndexBufferCore::initialize */
		void initialize() override;
	};

	/** @} */
}