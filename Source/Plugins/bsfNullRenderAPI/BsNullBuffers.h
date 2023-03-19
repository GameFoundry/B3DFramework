//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Allocators/BsPoolAlloc.h"
#include "RenderAPI/BsGenericGpuBuffer.h"
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
			SPtr<VertexBuffer> CreateVertexBufferInternal(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<IndexBuffer> CreateIndexBufferInternal(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer) override;
		};

		/**	Class containing common functionality for all Null hardware buffers. */
		class NullHardwareBuffer final : public GpuBuffer
		{
		public:
			NullHardwareBuffer(GpuBufferUsage usage, u32 elementCount, u32 elementSize);

			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override {}
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override {}
			void CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override {}

		protected:
			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx) override;
			void Unmap() override;

			void* mStagingBuffer = nullptr;
		};

		/**	Null implementation of a generic GPU buffer. */
		class NullGpuBuffer final : public GpuBuffer
		{
		public:
			NullGpuBuffer(const GpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask);
			NullGpuBuffer(const GpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer);

		protected:
			friend class NullHardwareBufferManager;

			void Initialize() override;
		};

		/**	Null implementation of a parameter block buffer (constant buffer in DX11 lingo). */
		class NullGpuParamBlockBuffer final : public GpuBuffer
		{
		public:
			NullGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);

		protected:
			void Initialize() override;
		};

		/**	Null implementation of an index buffer. */
		class NullIndexBuffer final : public IndexBuffer
		{
		public:
			NullIndexBuffer(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask);

		protected:
			void Initialize() override;
		};

		/**	Null implementation of a vertex buffer. */
		class NullVertexBuffer final : public VertexBuffer
		{
		public:
			NullVertexBuffer(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask);

		protected:
			void Initialize() override;
		};

		/** @} */
	} // namespace ct
} // namespace bs

namespace bs
{
	B3D_IMPLEMENT_GLOBAL_POOL(ct::NullHardwareBuffer, 32)
}
