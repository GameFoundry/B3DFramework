//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsHardwareBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a Vulkan buffer object that manages its usage and lifetime. */
	class VulkanBuffer : public VulkanResource
	{
	public:
		VulkanBuffer(VulkanResourceManager* owner, VkBuffer buffer, VkBufferView view, VkDeviceMemory memory);
		~VulkanBuffer();

		/** Returns the internal handle to the Vulkan object. */
		VkBuffer getHandle() const { return mBuffer; }

		/** Returns a buffer view that covers the entire buffer. */
		VkBufferView getView() const { return mView; }

	private:
		VkBuffer mBuffer;
		VkBufferView mView;
		VkDeviceMemory mMemory;
	};
	
	/**	Class containing common functionality for all Vulkan hardware buffers. */
	class VulkanHardwareBuffer : public HardwareBuffer
	{
	public:
		/**	Available types of Vulkan buffers. */
		enum BufferType
		{
			/** Contains geometry vertices and their properties. */
			BT_VERTEX = 0x1,
			/** Contains triangle to vertex mapping. */
			BT_INDEX = 0x2,
			/** Contains GPU program parameters. */
			BT_UNIFORM = 0x4,
			/** Generic read-only GPU buffer containing formatted data. */
			BT_GENERIC = 0x8,
			/** Generic read/write GPU buffer containing formatted data. */
			BT_STORAGE = 0x10,
			/** Helper buffer that can be written to on the CPU. Used for copy operations. */
			BT_STAGING = 0x20,
		};

		VulkanHardwareBuffer(BufferType type, GpuBufferFormat format, GpuBufferUsage usage, UINT32 size,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);
		~VulkanHardwareBuffer();

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 queueIdx = 1) override;

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 1) override;

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, 
			UINT32 length, bool discardWholeBuffer = false, UINT32 queueIdx = 1) override;

		/** 
		 * Gets the resource wrapping the buffer object, on the specified device. If hardware buffer device mask doesn't 
		 * include the provided device, null is returned. 
		 */
		VulkanBuffer* getResource(UINT32 deviceIdx) const { return mBuffers[deviceIdx]; }

	protected:
		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 queueIdx) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		VulkanBuffer* mBuffers[BS_MAX_DEVICES];
	};

	/** @} */
}