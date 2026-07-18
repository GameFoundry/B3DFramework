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

		/**
		 * Wraps a native D3D12 buffer resource and its memory allocation. Lifetime is owned by the device's
		 * resource manager and released via IGpuResource::Destroy(), deferred until the GPU is done with the
		 * resource.
		 *
		 * The buffer intentionally stores NO native resource state: D3D12 buffers decay to COMMON when each
		 * ExecuteCommandLists completes and implicitly promote from COMMON on first use, so states only exist
		 * per command buffer and are tracked by D3D12ResourceTracker.
		 */
		class D3D12Buffer : public TD3D12Resource<IGpuBufferResource>
		{
		public:
			D3D12Buffer(D3D12ResourceManager* owner, ComPtr<ID3D12Resource> resource, D3D12MA::Allocation* allocation,
				D3D12_HEAP_TYPE heapType, const StringView& name = "");
			~D3D12Buffer() override;

			/** Returns the native D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const { return mResource.Get(); }

			/** Returns the GPU virtual address of the buffer. */
			D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return mResource != nullptr ? mResource->GetGPUVirtualAddress() : 0; }

			/**
			 * Returns the heap the buffer's memory lives in. UPLOAD heap buffers are permanently in the GENERIC_READ
			 * state and READBACK heap buffers permanently in COPY_DEST - neither can be transitioned.
			 */
			D3D12_HEAP_TYPE GetHeapType() const { return mHeapType; }

		private:
			ComPtr<ID3D12Resource> mResource;
			D3D12MA::Allocation* mAllocation = nullptr;
			D3D12_HEAP_TYPE mHeapType = D3D12_HEAP_TYPE_DEFAULT;
		};

		/** DirectX 12 implementation of a GPU buffer. */
		class D3D12GpuBuffer : public GpuBuffer
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
			u32 GetBoundCount() const override;

			/** @copydoc IGpuResource::GetUseCount */
			u32 GetUseCount() const override;

			/** @copydoc render::GpuBuffer::Flush */
			void Flush(u32 offset, u32 size) override;

			/** @copydoc render::GpuBuffer::Invalidate */
			void Invalidate(u32 offset, u32 size) override;

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override;
			bool IsRangeInUse(u32 offset, u32 size) const override;
#endif

			/** Returns the low-level buffer resource wrapping the native D3D12 buffer. */
			D3D12Buffer* GetD3D12Buffer() const { return mBuffer; }

			/** Returns the D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const { return mBuffer != nullptr ? mBuffer->GetD3D12Resource() : nullptr; }

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
			 * structured/typed/byte storage buffer. For simple storage buffers @p format overrides the element format
			 * the view interprets the contents as (BF_UNKNOWN uses the buffer's own format); other buffer types ignore
			 * it. Returns a zeroed handle if the buffer cannot be viewed as an SRV.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle(GpuBufferFormat format = BF_UNKNOWN) const;

			/**
			 * Returns a CPU descriptor handle for an unordered access view (UAV) of the buffer, viewing it as a writable
			 * storage buffer. For simple storage buffers @p format overrides the element format the view interprets the
			 * contents as (BF_UNKNOWN uses the buffer's own format); other buffer types ignore it. Returns a zeroed
			 * handle if the buffer was not created with AllowUnorderedAccessOnTheGPU.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetUAVHandle(GpuBufferFormat format = BF_UNKNOWN) const;

		protected:
			/** @copydoc render::GpuBuffer::RecreateInternalBuffer */
			void RecreateInternalBuffer() override;

		private:
			/** SRV/UAV pair viewing a simple storage buffer's contents through an overridden element format. */
			struct FormatOverrideViews
			{
				GpuBufferFormat Format = BF_UNKNOWN;
				D3D12_CPU_DESCRIPTOR_HANDLE Srv{};
				D3D12_CPU_DESCRIPTOR_HANDLE Uav{};
			};

			/** Queues the current D3D12Buffer (if any) for deferred destruction, unmapping it and dropping views first. */
			void ReleaseBuffer();

			/** Creates the CPU shader-binding descriptors (CBV/SRV/UAV) valid for this buffer's type and flags. */
			void CreateShaderDescriptors();

			/** Frees any previously created shader-binding descriptors. */
			void ReleaseShaderDescriptors();

			/**
			 * Returns a cached (or lazily created) SRV/UAV of a simple storage buffer reinterpreted through @p format.
			 * Returns a zeroed handle if the format has no typed-buffer DXGI equivalent, or a UAV is requested but the
			 * buffer doesn't allow unordered access.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetFormatOverrideView(GpuBufferFormat format, bool readWrite) const;

			D3D12Buffer* mBuffer = nullptr;

			D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
			D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

			// CPU-only descriptor handles for shader binding. A zeroed ptr indicates the view is not applicable to this
			// buffer's type/flags, or hasn't been created.
			D3D12_CPU_DESCRIPTOR_HANDLE mCBVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mSRVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mUAVHandle{};

			// Views for bindings that override the element format (e.g. GpuSort moving 16X2U payloads as raw uints),
			// created on first use per format.
			mutable Vector<FormatOverrideViews> mFormatViews;
			mutable Mutex mViewMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
