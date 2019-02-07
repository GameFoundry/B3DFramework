//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Allocators/BsPoolAlloc.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup NullRenderAPI
	 *  @{
	 */

	/**	Handles creation of hardware buffers. */
	class NullHardwareBufferManager final : public HardwareBufferManager
	{
	protected:     
		/** @copydoc HardwareBufferManager::createVertexBufferInternal */
		SPtr<VertexBuffer> createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createIndexBufferInternal */
		SPtr<IndexBuffer> createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuParamBlockBufferInternal  */
		SPtr<GpuParamBlockBuffer> createGpuParamBlockBufferInternal(UINT32 size, 
			GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC&, GpuDeviceFlags) */
		SPtr<GpuBuffer> createGpuBufferInternal(const GPU_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC&, SPtr<HardwareBuffer>) */
		SPtr<GpuBuffer> createGpuBufferInternal(const GPU_BUFFER_DESC& desc, 
			SPtr<HardwareBuffer> underlyingBuffer) override;
	};

	/**	Class containing common functionality for all Null hardware buffers. */
	class NullHardwareBuffer final : public HardwareBuffer
	{
	public:
		NullHardwareBuffer(GpuBufferUsage usage, UINT32 elementCount, UINT32 elementSize);

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override { }

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override { }

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

	protected:
		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		void* mStagingBuffer = nullptr;
	};

	/**	Null implementation of a generic GPU buffer. */
	class NullGpuBuffer final : public GpuBuffer
	{
	public:
		NullGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		NullGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer);

	protected:
		friend class NullHardwareBufferManager;

		/** @copydoc GpuBuffer::initialize */
		void initialize() override;
	};

	/**	Null implementation of a parameter block buffer (constant buffer in DX11 lingo). */
	class NullGpuParamBlockBuffer final : public GpuParamBlockBuffer
	{
	public:
		NullGpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);

	protected:
		/** @copydoc GpuParamBlockBuffer::initialize */
		void initialize() override;
	};

	/**	Null implementation of an index buffer. */
	class NullIndexBuffer final : public IndexBuffer
	{
	public:
		NullIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

	protected:
		/** @copydoc IndexBuffer::initialize */
		void initialize() override;
	};

	/**	Null implementation of a vertex buffer. */
	class NullVertexBuffer final : public VertexBuffer
	{
	public:
		NullVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

	protected: 
		/** @copydoc VertexBuffer::initialize */
		void initialize() override;
	};

	/** @} */
}}

namespace bs
{
	IMPLEMENT_GLOBAL_POOL(ct::NullHardwareBuffer, 32)
}