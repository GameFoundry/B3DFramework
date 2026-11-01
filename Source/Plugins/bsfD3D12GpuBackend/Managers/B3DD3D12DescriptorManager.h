//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "Allocators/B3DRangeAllocator.h"
#include "GpuBackend/B3DGpuBindlessHeapPool.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Descriptor heap type. */
		enum class D3D12DescriptorHeapType
		{
			CBV_SRV_UAV,	/**< Constant buffer, shader resource, and unordered access views. */
			Sampler,		/**< Sampler descriptors. */
			RTV,			/**< Render target views. */
			DSV				/**< Depth-stencil views. */
		};

		class D3D12DescriptorManager;

		/**
		 * Shader-visible CBV/SRV/UAV heap: the bindless resource region of GetCapacity() descriptors at the start of the
		 * heap, so a bindless index equals its heap index, followed by the table region. See GpuBindlessHeapPool.
		 */
		class D3D12BindlessHeap : public TD3D12Resource<GpuBindlessHeap>
		{
		public:
			D3D12BindlessHeap(D3D12ResourceManager* owner, u32 capacity, ComPtr<ID3D12DescriptorHeap> heap, u32 descriptorSize);

			/** Returns the native shader-visible heap. */
			ID3D12DescriptorHeap* GetHeap() const { return mHeap.Get(); }

			/** Returns the CPU handle of descriptor @p index of the heap. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(u32 index) const { return D3D12_CPU_DESCRIPTOR_HANDLE{ mCPUStart.ptr + (SIZE_T)index * mDescriptorSize }; }

			/** Returns the GPU handle of descriptor @p index of the heap. */
			D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(u32 index) const { return D3D12_GPU_DESCRIPTOR_HANDLE{ mGPUStart.ptr + (u64)index * mDescriptorSize }; }

			/** Returns the heap index of the first descriptor of the table region. */
			u32 GetTableRegionStart() const { return GetCapacity(); }

		private:
			ComPtr<ID3D12DescriptorHeap> mHeap;
			D3D12_CPU_DESCRIPTOR_HANDLE mCPUStart;
			D3D12_GPU_DESCRIPTOR_HANDLE mGPUStart;
			u32 mDescriptorSize;
		};

		/**
		 * Tracked allocation within the table region of a shader-visible descriptor heap.
		 *
		 * Every shader-visible CBV/SRV/UAV heap has a table region, and tables have the same offset in each. A table
		 * is written into the heap it is first bound with, and copied from its source descriptors into other heaps when bound with them.
		 */
		class D3D12DescriptorTable : public D3D12Resource
		{
		public:
			D3D12DescriptorTable(D3D12ResourceManager* owner, D3D12DescriptorManager& descriptorManager, D3D12DescriptorHeapType type, u32 offset, u32 descriptorCount);
			~D3D12DescriptorTable() override;

			/**
			 * Writes @p sourceHandles, one per descriptor of the table, into the table, as seen through @p heap. Other heaps copy them
			 * when the table is bound with them. Must only be called while no command buffer binds the table.
			 *
			 * @param	sourceHandles	CPU-only descriptors to copy. Must stay valid while the table may be bound.
			 * @param	heap			Heap to write the CBV/SRV/UAV descriptors into. Ignored for sampler tables.
			 */
			void Write(TArrayView<const D3D12_CPU_DESCRIPTOR_HANDLE> sourceHandles, const D3D12BindlessHeap* heap);

			/**
			 * Returns the GPU handle of the first descriptor of the table, as seen through @p heap, copying the table's descriptors into
			 * @p heap first if it lacks them. @p heap is ignored for sampler tables.
			 */
			D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const D3D12BindlessHeap* heap);

		private:
			/** Copies the source descriptors into the table region of @p heap. Must be called with mMutex held. */
			void CopyInto(const D3D12BindlessHeap* heap);

			D3D12DescriptorManager& mDescriptorManager;
			D3D12DescriptorHeapType mType;
			u32 mOffset; /**< First descriptor, relative to the start of the table region. */
			u32 mDescriptorCount;
			Vector<D3D12_CPU_DESCRIPTOR_HANDLE> mSourceHandles;
			TInlineArray<u64, 4> mHeapVersions; /**< Versions of the CBV/SRV/UAV heaps holding the current descriptors. */
			Mutex mMutex;
		};

		/**
		 * Manages allocation of descriptor heaps and individual descriptors.
		 *
		 * Resource views live in CPU-only staging heaps; parameter sets copy them into tracked, contiguous shader-visible
		 * tables before binding. The two must not share a heap, as shader-visible heaps cannot be a descriptor copy source.
		 *
		 * The shader-visible CBV/SRV/UAV heaps are pooled (see GpuBindlessHeapPool), so bindless slots the GPU may be reading can
		 * change, and the bindless region can grow. A CPU-only mirror of the bindless region holds the latest descriptor of every slot,
		 * and is the source heaps are filled from. There is a single shader-visible sampler heap.
		 *
		 * @note Thread safe.
		 */
		class D3D12DescriptorManager : private IGpuBindlessHeapPoolBackend
		{
		public:
			/**
			 * Creates the descriptor heaps and immutable fallback descriptors for @p device. The shader-visible heaps start with
			 * bindless regions of @p bindlessResourceCount and @p bindlessSamplerCount descriptors, followed by the table regions.
			 */
			D3D12DescriptorManager(D3D12GpuDevice& device, u32 bindlessResourceCount, u32 bindlessSamplerCount);
			~D3D12DescriptorManager() override;

			/** Allocates a descriptor from the CPU-only staging heap of the specified type. */
			D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPUDescriptor(D3D12DescriptorHeapType type);

			/** Frees a previously allocated descriptor. */
			void FreeCPUDescriptor(D3D12DescriptorHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/**
			 * Allocates a tracked contiguous range of GPU-visible descriptors. Only valid for the CBV_SRV_UAV and Sampler
			 * heap types. Returns null when no free range can satisfy @p count.
			 */
			D3D12DescriptorTable* AllocateGPUDescriptorTable(D3D12DescriptorHeapType type, u32 count);

			/** Returns the shader-visible sampler heap. CBV/SRV/UAV heaps are pooled, see GetAndTrackResourceHeap(). */
			ID3D12DescriptorHeap* GetSamplerHeap() const { return mHeaps[(u32)D3D12DescriptorHeapType::Sampler].Heap.Get(); }

			/**
			 * Returns the current shader-visible CBV/SRV/UAV heap, and tracks it with @p resourceTracker, which keeps it
			 * alive and unchanged while the command buffer owning the tracker is recording or executing.
			 */
			D3D12BindlessHeap* GetAndTrackResourceHeap(GpuResourceTracker& resourceTracker);

			/** Returns the current shader-visible CBV/SRV/UAV heap. Tables written through it reach command buffers that get it. */
			D3D12BindlessHeap* GetCurrentResourceHeap() const;

			/** Returns the pool of shader-visible CBV/SRV/UAV heaps. */
			GpuBindlessHeapPool& GetResourceHeapPool() { return *mResourceHeapPool; }

			/** Destroys every shader-visible CBV/SRV/UAV heap. Must be called before the resource manager is destroyed, once no command buffer binds them. */
			void DestroyResourceHeapPool() { mResourceHeapPool = nullptr; }

			/** Writes @p source into bindless resource slot @p slotIndex of the mirror and of every heap. Nothing may read the slot. */
			void WriteBindlessResourceSlot(u32 slotIndex, D3D12_CPU_DESCRIPTOR_HANDLE source);

			/** Writes @p source into slot @p slotIndex of the bindless region of the shader-visible sampler heap. Nothing may read the slot. */
			void WriteBindlessSamplerSlot(u32 slotIndex, D3D12_CPU_DESCRIPTOR_HANDLE source);

			/** Grows the bindless resource region to @p capacity descriptors, see GpuBindlessHeapPool::Grow(). Returns false on failure. */
			bool GrowBindlessResourceRegion(u32 capacity, const TBitfield<>& liveSlots);

			/**
			 * Returns a CPU descriptor for the default sampler (linear filtering, wrap addressing), used for sampler
			 * bindings the caller never set. Sampler heap slots have no null descriptor, so unset bindings must fall
			 * back to something valid or the shader samples through an uninitialized descriptor.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultSamplerCPUHandle() const { return mDefaultSamplerHandle; }

			/**
			 * Returns a null CBV descriptor, used for uniform buffer bindings the caller never set. Shader reads
			 * through it return zero. Copying nothing into a shader-visible heap slot is not an alternative: freshly
			 * allocated ranges contain whatever the previous frame left there.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetNullCBVHandle() const { return mNullCBVHandle; }

			/**
			 * Returns a null SRV descriptor for a read-only texture or buffer that shaders read as @p type, used for bindings the
			 * caller never set and for retired bindless slots. Shader reads through it return zero. The view dimension, and the
			 * stride and flags of buffer views, match @p type, as the null-descriptor guarantees only hold for a matching view.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetNullSRVHandle(GpuParameterObjectType type) const;

			/**
			 * Returns a null UAV descriptor for a read-write texture or buffer that shaders access as @p type, used for bindings
			 * the caller never set. Shader reads through it return zero, writes are dropped. The view matches @p type, as for
			 * GetNullSRVHandle().
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetNullUAVHandle(GpuParameterObjectType type) const;

			/** Returns the stride between consecutive descriptors in a heap of the specified type. */
			u32 GetDescriptorSize(D3D12DescriptorHeapType type) const { return mDescriptorSizes[(u32)type]; }

			/** Number of descriptors in the table region of every shader-visible CBV/SRV/UAV heap. */
			static constexpr u32 kResourceTableDescriptorCount = 64 * 1024;

			/** Minimum number of descriptors in the table region of the shader-visible sampler heap. The bindless region takes the rest. */
			static constexpr u32 kSamplerTableDescriptorCount = 1024;

		private:
			friend class D3D12DescriptorTable;

			/** Descriptor heap for a specific type. */
			struct DescriptorHeap
			{
				ComPtr<ID3D12DescriptorHeap> Heap;		/**< Native descriptor heap. Null for the CBV/SRV/UAV heaps, which mResourceHeapPool owns. */
				D3D12_CPU_DESCRIPTOR_HANDLE CPUStart{};	/**< CPU handle of the first descriptor. */
				D3D12_GPU_DESCRIPTOR_HANDLE GPUStart{};	/**< Only valid for shader-visible heaps. */
				u32 DescriptorCount = 0;				/**< Capacity the heap was created with. */
				u32 TableStart = 0;						/**< First descriptor of the table region of a shader-visible heap. Bindless descriptors precede it. */
				u32 NextFreeIndex = 0;					/**< Bump-allocation cursor into the heap. */
				Vector<u32> FreeList;					/**< Indices returned by FreeCPUDescriptor(), reused before the cursor advances. */
				RangeAllocator FreeRanges;				/**< Free descriptors of the table region of a shader-visible heap, relative to TableStart. */
				Mutex AllocationMutex;					/**< Guards descriptor allocation and release. */
			};

			/** Creates the descriptor heaps, both the shader-visible ones and the CPU-only staging heaps. */
			void CreateHeaps(u32 bindlessResourceCount, u32 bindlessSamplerCount);

			/** Replaces the bindless resource mirror with one of @p capacity descriptors, keeping the descriptors of the old one. Returns false on failure. */
			bool ResizeBindlessResourceMirror(u32 capacity);

			/** Returns the CPU handle of slot @p slotIndex of the bindless resource mirror. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetMirrorHandle(u32 slotIndex) const;

			/**
			 * @name IGpuBindlessHeapPoolBackend implementation
			 * @{
			 */

			GpuBindlessHeap* CreateHeap(u32 capacity) override;
			void CopySlots(GpuBindlessHeap& destination, const GpuBindlessHeap& current, TArrayView<const TIndexRange<u32>> ranges) override;
			void WriteUpdates(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates) override;

			/** @} */

			/** Creates the null CBV/SRV/UAV descriptors unset resource bindings fall back to. */
			void CreateNullDescriptors();

			/** Returns a shader-visible range after the tracked table using it is no longer bound or in flight. */
			void FreeGPUDescriptorRange(D3D12DescriptorHeapType type, u32 startIndex, u32 descriptorCount);

			/**
			 * Returns the CPU-only staging heap for the given type. RTV/DSV heaps are CPU-only to begin with, while
			 * CBV_SRV_UAV/Sampler have dedicated staging heaps alongside their shader-visible ones.
			 */
			DescriptorHeap& GetStagingHeap(D3D12DescriptorHeapType type);

			/** Number of D3D12DescriptorHeapType values. */
			static constexpr u32 kHeapTypeCount = 4;

			/**
			 * Number of heap types that need a separate CPU-only staging heap. Indexed by D3D12DescriptorHeapType,
			 * which relies on CBV_SRV_UAV and Sampler being the first two enum values.
			 */
			static constexpr u32 kStagingHeapTypeCount = 2;

			D3D12GpuDevice& mDevice;
			DescriptorHeap mHeaps[kHeapTypeCount];					/**< Shader-visible Sampler heap, table region allocation of the CBV_SRV_UAV heaps, CPU-only RTV/DSV heaps. */
			TUnique<GpuBindlessHeapPool> mResourceHeapPool;	/**< Shader-visible CBV_SRV_UAV heaps. */
			ComPtr<ID3D12DescriptorHeap> mBindlessResourceMirror;	/**< CPU-only copy of the bindless region, holding the latest descriptor of every slot. Null without bindless support. */
			u32 mBindlessResourceMirrorCapacity = 0;
			DescriptorHeap mStagingHeaps[kStagingHeapTypeCount];	/**< CPU-only staging heaps for CBV_SRV_UAV and Sampler resource views */
			u32 mDescriptorSizes[kHeapTypeCount] = {};				/**< Descriptor size for each type. */

			// Fallbacks for resource bindings never set by the caller, indexed by view dimension where applicable
			D3D12_CPU_DESCRIPTOR_HANDLE mDefaultSamplerHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullCBVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURECUBEARRAY + 1] = {}; /**< Indexed by dimension. Typed views for buffers. */
			D3D12_CPU_DESCRIPTOR_HANDLE mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE3D + 1] = {}; /**< Indexed by dimension. Typed views for buffers. */
			D3D12_CPU_DESCRIPTOR_HANDLE mNullStructuredBufferSRVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullStructuredBufferUAVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullRawBufferSRVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullRawBufferUAVHandle{};
		};

		/** @} */
	} // namespace render
} // namespace b3d
