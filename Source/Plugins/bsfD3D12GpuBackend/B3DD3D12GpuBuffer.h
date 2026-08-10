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
		 * Wraps a slice of a pooled native D3D12 buffer resource. Lifetime is owned by the device's
		 * resource manager and released via IGpuResource::Destroy(), deferred until the GPU is done with the
		 * resource.
		 *
		 * Logical hazards are tracked by the core resource tracker; backing-page write hazards are added by the
		 * D3D12 barrier helper.
		 */
		class D3D12Buffer : public TD3D12Resource<IGpuBufferResource>
		{
		public:
			/** Creates a logical buffer owning @p allocation until its tracked GPU uses complete. */
			D3D12Buffer(D3D12ResourceManager* owner, GpuResourceLocation allocation, const StringView& name = "");
			~D3D12Buffer() override;

			/** Returns the native D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const;

			/** Returns the GPU virtual address of the buffer. */
			D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

			/** Returns the native page shared by this logical buffer and other compatible slices. */
			D3D12BufferPage* GetPage() const;

			/** Returns the byte offset of this logical buffer from the start of its native page. */
			u64 GetOffset() const { return mAllocation.Offset; }

			/** Returns the heap type of the shared native page containing this buffer slice. */
			D3D12_HEAP_TYPE GetHeapType() const;

		private:
			GpuResourceLocation mAllocation;
		};

		/** DirectX 12 implementation of a GPU buffer. */
		class D3D12GpuBuffer : public GpuBuffer
		{
			/** Type of shader-binding descriptor associated with a buffer view. */
			enum class ViewType
			{
				CBV, /**< Constant buffer view. */
				SRV, /**< Shader resource view. */
				UAV  /**< Unordered access view. */
			};

			/** Shader-binding descriptors viewing the buffer through a particular element format. */
			struct BufferViews
			{
				BufferViews() = default;
				explicit BufferViews(GpuBufferFormat format) : Format(format) { }

				GpuBufferFormat Format = BF_UNKNOWN; /**< Element format shared by these descriptors. */
				D3D12_CPU_DESCRIPTOR_HANDLE Cbv{};   /**< Constant buffer view, when applicable. */
				D3D12_CPU_DESCRIPTOR_HANDLE Srv{};   /**< Shader resource view, when applicable. */
				D3D12_CPU_DESCRIPTOR_HANDLE Uav{};   /**< Unordered access view, when applicable. */
			};

		public:
			/** Creates an uninitialized D3D12 buffer; Initialize() allocates its native slice. */
			D3D12GpuBuffer(const GpuBufferCreateInformation& createInformation, GpuDevice& device);
			~D3D12GpuBuffer() override;

			void Initialize() override;
			void SetName(const StringView& name) override;
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override;
			u32 GetBoundCount() const override;
			u32 GetUseCount() const override;

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
			 * @param	suballocationIndex	Zero-based suballocation to view. Values other than 0 are not yet supported.
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
			void RecreateInternalBuffer() override;

		private:
			/** Queues the current D3D12Buffer for deferred destruction after dropping its mapped pointer and descriptors. */
			void ReleaseBuffer();

			/**
			 * Returns a cached descriptor of @p type, creating it if needed. Typed simple-storage views are keyed by
			 * @p format; other buffer types use BF_UNKNOWN. Returns a zeroed handle when the requested view isn't valid.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetOrCreateView(GpuBufferFormat format, ViewType type) const;

			D3D12Buffer* mBuffer = nullptr;

			D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
			D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

			mutable TInlineArray<BufferViews, 2> mViews; /**< Default descriptors plus typed overrides created on demand. Most buffers need at most one override. */
			mutable Mutex mViewMutex; /**< Guards descriptor lookup, creation, and release. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
