//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Allocators/BsPoolAlloc.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup NullRenderAPI
		 *  @{
		 */

		/**	Handles creation of hardware buffers. */
		class NullHardwareBufferManager final : public HardwareBufferManager
		{
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
		};

		/**	Class containing common functionality for all Null hardware buffers. */
		class NullHardwareBuffer final : public HardwareBuffer
		{
		public:
			NullHardwareBuffer(GpuBufferUsage usage, u32 elementCount, u32 elementSize);

			/** @copydoc HardwareBuffer::readData */
			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override {}

			/** @copydoc HardwareBuffer::writeData */
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override {}

			/** @copydoc HardwareBuffer::copyData */
			void CopyData(HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

		protected:
			/** @copydoc HardwareBuffer::map */
			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx) override;

			/** @copydoc HardwareBuffer::unmap */
			void Unmap() override;

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
			void Initialize() override;
		};

		/**	Null implementation of a parameter block buffer (constant buffer in DX11 lingo). */
		class NullGpuParamBlockBuffer final : public GpuParamBlockBuffer
		{
		public:
			NullGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);

		protected:
			/** @copydoc GpuParamBlockBuffer::initialize */
			void Initialize() override;
		};

		/**	Null implementation of an index buffer. */
		class NullIndexBuffer final : public IndexBuffer
		{
		public:
			NullIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		protected:
			/** @copydoc IndexBuffer::initialize */
			void Initialize() override;
		};

		/**	Null implementation of a vertex buffer. */
		class NullVertexBuffer final : public VertexBuffer
		{
		public:
			NullVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		protected:
			/** @copydoc VertexBuffer::initialize */
			void Initialize() override;
		};

		/** @} */
	} // namespace ct
} // namespace bs

namespace bs
{
	IMPLEMENT_GLOBAL_POOL(ct::NullHardwareBuffer, 32)
}
