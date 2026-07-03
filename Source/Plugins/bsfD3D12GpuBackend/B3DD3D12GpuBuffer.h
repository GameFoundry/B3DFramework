//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "B3DD3D12Resource.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
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

			/** @copydoc render::GpuBuffer::SetName */
			void SetName(const StringView& name) override;

			/** @copydoc render::GpuBuffer::GetUseMask */
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override;

			/** @copydoc IGpuResource::GetBoundCount */
			u32 GetBoundCount() const override { return D3D12Resource::GetBoundCount(); }

			/** @copydoc IGpuResource::GetUseCount */
			u32 GetUseCount() const override { return D3D12Resource::GetUseCount(); }

			/** @copydoc render::GpuBuffer::Flush */
			void Flush(u32 offset, u32 size) override;

			/** @copydoc render::GpuBuffer::Invalidate */
			void Invalidate(u32 offset, u32 size) override;

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override { return false; }
			bool IsRangeInUse(u32 offset, u32 size) const override { return false; }
#endif

			/** Returns the D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const override { return mBuffer.Get(); }

			/** Returns the GPU virtual address of the buffer. */
			D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

			/** Returns the vertex buffer view (only valid for vertex buffers). */
			const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return mVertexBufferView; }

			/** Returns the index buffer view (only valid for index buffers). */
			const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return mIndexBufferView; }

			/**
			 * Returns a CPU descriptor handle for a constant buffer view (CBV) of the specified suballocation. Only valid
			 * for uniform buffers. Returns a zeroed handle if the buffer is not a uniform buffer, or if descriptor
			 * allocation failed.
			 *
			 * @param[in]	suballocationIndex	Zero-based suballocation to view. Values other than 0 are not yet supported.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetCBVHandle(u32 suballocationIndex = 0) const;

			/**
			 * Returns a CPU descriptor handle for a shader resource view (SRV) of the buffer, viewing it as a read-only
			 * structured/typed/byte storage buffer. Returns a zeroed handle if the buffer cannot be viewed as an SRV.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle() const;

			/**
			 * Returns a CPU descriptor handle for an unordered access view (UAV) of the buffer, viewing it as a writable
			 * storage buffer. Returns a zeroed handle if the buffer was not created with AllowUnorderedAccessOnTheGPU.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetUAVHandle() const;

		protected:
			/** @copydoc render::GpuBuffer::RecreateInternalBuffer */
			void RecreateInternalBuffer() override;

		private:
			/** Releases the underlying D3D12 resource and (if mapped) unmaps its persistent mapping. */
			void ReleaseBuffer();

			/** Creates the CPU shader-binding descriptors (CBV/SRV/UAV) valid for this buffer's type and flags. */
			void CreateShaderDescriptors();

			/** Frees any previously created shader-binding descriptors. */
			void ReleaseShaderDescriptors();

			ComPtr<ID3D12Resource> mBuffer;
			D3D12MA::Allocation* mAllocation = nullptr;

			D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
			D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

			// CPU-only descriptor handles for shader binding. A zeroed ptr indicates the view is not applicable to this
			// buffer's type/flags, or hasn't been created.
			D3D12_CPU_DESCRIPTOR_HANDLE mCBVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mSRVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mUAVHandle{};
		};

		/** @} */
	} // namespace render
} // namespace b3d
