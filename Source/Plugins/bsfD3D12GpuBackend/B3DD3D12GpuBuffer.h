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
		 * Wraps a native D3D12 buffer: either a slice of a pooled native buffer resource, or a placed resource of its own in
		 * a native heap. Lifetime is owned by the device's resource manager and released via IGpuResource::Destroy(),
		 * deferred until the GPU is done with the resource.
		 */
		class D3D12Buffer : public D3D12BufferResource
		{
		public:
			/** Creates a logical buffer on a slice of a buffer page, owning @p allocation until its tracked GPU uses complete. */
			D3D12Buffer(D3D12ResourceManager* owner, GpuAllocation allocation, const StringView& name = "");

			/**
			 * Creates a buffer that owns @p resource, placed at @p allocation in a native heap of type @p heapType. Owns
			 * @p allocation until its tracked GPU uses complete, if the allocation is owned. @p mappedData is the persistent
			 * CPU mapping of the resource, or null.
			 */
			D3D12Buffer(D3D12ResourceManager* owner, GpuAllocation allocation, ComPtr<ID3D12Resource> resource, D3D12_HEAP_TYPE heapType, void* mappedData,
				const StringView& name = "");
			~D3D12Buffer() override;

			/** Returns the native D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const override;

			/** Returns the GPU virtual address of the buffer. */
			D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

			/** Returns the native page shared by this buffer slice and other compatible slices, or null for a placed buffer. */
			D3D12BufferPage* GetPage() const override;

			/** Returns the byte offset of this buffer from the start of its native resource. */
			u64 GetOffset() const { return mResource != nullptr ? 0 : mAllocation.Offset; }

			/** Returns the native heap type of the memory the buffer is in. */
			D3D12_HEAP_TYPE GetHeapType() const override;

			/** Returns the persistent CPU mapping of the buffer, or null for device-local buffers. */
			void* GetMappedData() const;

		private:
			GpuAllocation mAllocation;
			ComPtr<ID3D12Resource> mResource; /**< Placed resource, or null if the buffer is a slice of a page. */
			D3D12_HEAP_TYPE mHeapType = D3D12_HEAP_TYPE_DEFAULT; /**< Heap type of the placed resource. */
			void* mMappedData = nullptr; /**< Persistent CPU mapping of the placed resource. */
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

			/** Shader-binding descriptors viewing the buffer through a particular element format and byte offset. */
			struct BufferViews
			{
				BufferViews() = default;
				BufferViews(GpuBufferFormat format, u32 offset) : Format(format), Offset(offset) { }

				GpuBufferFormat Format = BF_UNKNOWN; /**< Element format shared by these descriptors. */
				u32 Offset = 0; /**< Byte offset for constant buffer views; zero for storage buffer views. */
				D3D12_CPU_DESCRIPTOR_HANDLE Cbv{};   /**< Constant buffer view, when applicable. */
				D3D12_CPU_DESCRIPTOR_HANDLE Srv{};   /**< Shader resource view, when applicable. */
				D3D12_CPU_DESCRIPTOR_HANDLE Uav{};   /**< Unordered access view, when applicable. */
			};

		public:
			/** Creates an uninitialized D3D12 buffer whose native slice is placed at @p allocation. See GpuDevice::CreateGpuBuffer(). */
			D3D12GpuBuffer(const GpuBufferCreateInformation& createInformation, GpuDevice& device, const GpuAllocation& allocation);
			~D3D12GpuBuffer() override;

			void Initialize() override;
			void SetName(const StringView& name) override;
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override;
			u32 GetBoundCount() const override;
			u32 GetUseCount() const override;
			IGpuResource* GetGpuResource() const override { return mBuffer; }

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
			 * Returns a CPU descriptor handle for a constant buffer view (CBV) covering one suballocation's size at
			 * @p offset bytes. Only valid for uniform buffers. Returns a zeroed handle if the view is invalid or
			 * descriptor allocation failed. Descriptors are cached by offset.
			 *
			 * @param	offset	Byte offset from the start of the buffer, aligned to 256 bytes.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetCBVHandle(u32 offset = 0) const;

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

			/** Returns the size of the native slice backing a buffer described by @p information, of @p totalSize bytes. */
			static u32 GetSliceSize(const GpuBufferInformation& information, u32 totalSize);

			/**
			 * Returns an alignment that keeps the pooled slice valid for every native view and copy footprint the logical
			 * buffer may use. The copy alignment is included for every buffer so a later buffer-to-image operation never
			 * depends on how the buffer was originally classified.
			 */
			static u32 GetSliceAlignment(const GpuBufferInformation& information);

			/**
			 * Returns the description of a placed buffer resource of @p size bytes with @p flags. The same description must be
			 * used to query the resource's size and alignment, and to create it.
			 *
			 * @param	size				Size of the buffer, in bytes.
			 * @param	flags				Native resource flags of the buffer.
			 * @param	useTightAlignment	True to request tight alignment for the resource. Must only be set if the device
			 *								supports it.
			 */
			static D3D12_RESOURCE_DESC GetResourceDescription(u64 size, D3D12_RESOURCE_FLAGS flags, bool useTightAlignment);

		protected:
			void RecreateInternalBuffer() override;

		private:
			/** Queues the current D3D12Buffer for deferred destruction after dropping its mapped pointer and descriptors. */
			void ReleaseBuffer();

			/**
			 * Returns a cached descriptor of @p type, creating it if needed. Typed simple-storage views are keyed by
			 * @p format; other buffer types use BF_UNKNOWN. Constant buffer views also use @p offset in bytes; other views
			 * use zero. Returns a zeroed handle when the requested view isn't valid.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetOrCreateView(GpuBufferFormat format, ViewType type, u32 offset = 0) const;

			D3D12Buffer* mBuffer = nullptr;

			D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
			D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

			mutable TInlineArray<BufferViews, 2> mViews; /**< Default descriptors plus format and constant-buffer offset views created on demand. */
			mutable Mutex mViewMutex; /**< Guards descriptor lookup, creation, and release. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
