//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "B3DD3D12Resource.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
		 *  @{
		 */

		/** DirectX 12 implementation of a GPU buffer. */
		class D3D12GpuBuffer : public GpuBuffer, public D3D12Resource
		{
		public:
			D3D12GpuBuffer(const GpuBufferCreateInformation& createInformation, GpuDevice& device);
			~D3D12GpuBuffer() override;

			/** @copydoc GpuBuffer::Initialize */
			void Initialize() override;

			/** Returns the D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const override { return mBuffer.Get(); }

			/** Returns the GPU virtual address of the buffer. */
			D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

			/** Returns the vertex buffer view (only valid for vertex buffers). */
			const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return mVertexBufferView; }

			/** Returns the index buffer view (only valid for index buffers). */
			const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return mIndexBufferView; }

			void Flush(u32 offset, u32 size) override;
			void Invalidate(u32 offset, u32 size) override;

		protected:
			/** @copydoc GpuBuffer::Map */
			void* Map(u32 offset, u32 length, GpuLockOptions options) override;

			/** @copydoc GpuBuffer::Unmap */
			void Unmap() override;

			/** @copydoc GpuBuffer::WriteData */
			void WriteData(u32 offset, u32 length, const void* source, GpuBufferWriteFlags flags) override;

			/** @copydoc GpuBuffer::ReadData */
			void ReadData(u32 offset, u32 length, void* dest) override;

			/** @copydoc GpuBuffer::CopyData */
			void CopyData(GpuBuffer& destination, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer) override;

		private:
			ComPtr<ID3D12Resource> mBuffer;
			D3D12MA::Allocation* mAllocation = nullptr;

			D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
			D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

			void* mMappedData = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
