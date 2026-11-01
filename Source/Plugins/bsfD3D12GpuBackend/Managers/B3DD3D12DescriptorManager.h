//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "B3DD3D12DescriptorHeap.h"
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
		 * Shader-visible CBV/SRV/UAV heap: the bindless resource region of GetCapacity() descriptors at the start of the heap, so a
		 * bindless index equals its heap index, followed by the table region. A native heap of the bindless resource heap, see
		 * GpuBindlessHeapPool. Devices without bindless support have one, with an empty bindless region.
		 */
		class D3D12BindlessHeap : public TD3D12Resource<GpuBindlessHeap>
		{
		public:
			D3D12BindlessHeap(D3D12ResourceManager* owner, u32 capacity, D3D12DescriptorHeap heap);

			/** Returns the native heap. Its table region starts after the GetCapacity() bindless descriptors. */
			const D3D12DescriptorHeap& GetDescriptorHeap() const { return mHeap; }

			/**
			 * Returns the descriptors of the table region written while the heap was not current, which it must copy before it is current
			 * again. Guarded by the table region mutex, see D3D12DescriptorManager::GetResourceTableMutex().
			 */
			TBitfield<>& GetDirtyTableDescriptors() { return mDirtyTableDescriptors; }

			/** Marks @p count descriptors of the table region, starting at @p offset, as written while the heap was not current. */
			void MarkTableDescriptorsDirty(u32 offset, u32 count);

		private:
			D3D12DescriptorHeap mHeap;
			TBitfield<> mDirtyTableDescriptors;
		};

		/**
		 * Tracked allocation within the table region of the shader-visible descriptor heaps of one type. Every shader-visible heap of the type
		 * has a table region, and the table has the same offset in each. Written through D3D12DescriptorManager::WriteTable().
		 */
		class D3D12DescriptorTable : public D3D12Resource
		{
		public:
			D3D12DescriptorTable(D3D12ResourceManager* owner, D3D12DescriptorManager& descriptorManager, D3D12DescriptorHeapType type, u32 offset, u32 descriptorCount);
			~D3D12DescriptorTable() override;

			/** Returns the type of the descriptors in the table. */
			D3D12DescriptorHeapType GetType() const { return mType; }

			/** Returns the first descriptor of the table, relative to the start of the table region. */
			u32 GetOffset() const { return mOffset; }

			/** Returns the number of descriptors in the table. */
			u32 GetDescriptorCount() const { return mDescriptorCount; }

			/** Returns the GPU handle of the first descriptor of the table, as seen through @p heap, a shader-visible heap of the table's type. */
			D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const D3D12DescriptorHeap& heap) const { return heap.GetGPUHandle(heap.GetTableRegionStart() + mOffset); }

		private:
			D3D12DescriptorManager& mDescriptorManager;
			D3D12DescriptorHeapType mType;
			u32 mOffset;
			u32 mDescriptorCount;
		};

		/**
		 * Manages allocation of descriptor heaps and individual descriptors.
		 *
		 * Resource views live in CPU-only staging heaps; parameter sets copy them into tracked, contiguous shader-visible
		 * tables before binding. The two must not share a heap, as shader-visible heaps cannot be a descriptor copy source.
		 *
		 * The shader-visible CBV/SRV/UAV heaps are the native heaps of the bindless resource heap (see D3D12BindlessHeaps), which
		 * switch so bindless slots the GPU may be reading can change, and the bindless region can grow. A CPU-only mirror of the table
		 * region holds the latest descriptors of every table, and is the source native heaps are filled from when they become current.
		 * Tables are written into the current native heap only, as command buffers bind the one that is current when they record.
		 * There is a single shader-visible sampler heap.
		 *
		 * @note Thread safe.
		 */
		class D3D12DescriptorManager
		{
		public:
			/**
			 * Creates the descriptor heaps and immutable fallback descriptors for @p device. The shader-visible sampler heap starts with
			 * a bindless region of @p bindlessSamplerCount descriptors, followed by the table region. Without @p supportsBindlessResources,
			 * creates the only shader-visible CBV/SRV/UAV heap; otherwise D3D12BindlessHeaps creates them.
			 */
			D3D12DescriptorManager(D3D12GpuDevice& device, bool supportsBindlessResources, u32 bindlessSamplerCount);
			~D3D12DescriptorManager();

			/** Allocates a descriptor from the CPU-only staging heap of the specified type. */
			D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPUDescriptor(D3D12DescriptorHeapType type);

			/** Frees a previously allocated descriptor. */
			void FreeCPUDescriptor(D3D12DescriptorHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/**
			 * Allocates a tracked contiguous range of GPU-visible descriptors. Only valid for the CBV_SRV_UAV and Sampler
			 * heap types. Returns null when no free range can satisfy @p count.
			 */
			D3D12DescriptorTable* AllocateGPUDescriptorTable(D3D12DescriptorHeapType type, u32 count);

			/** Returns the range of @p table to the table region. Called by the table once no command buffer binds it. */
			void FreeGPUDescriptorTable(const D3D12DescriptorTable& table);

			/**
			 * Copies @p sources, one per descriptor of @p table, into the table. The sources are not referenced afterwards. No command
			 * buffer may bind the table.
			 */
			void WriteTable(const D3D12DescriptorTable& table, TArrayView<const D3D12_CPU_DESCRIPTOR_HANDLE> sources);

			/** Returns the shader-visible sampler heap. CBV/SRV/UAV heaps switch, see GetAndTrackResourceHeap(). */
			const D3D12DescriptorHeap& GetSamplerHeap() const { return mSamplerHeap; }

			/**
			 * Returns the current shader-visible CBV/SRV/UAV heap, and tracks it with @p resourceTracker, which keeps it
			 * alive and unchanged while the command buffer owning the tracker is recording or executing.
			 */
			D3D12BindlessHeap* GetAndTrackResourceHeap(GpuResourceTracker& resourceTracker);

			/** Returns the current shader-visible CBV/SRV/UAV heap. Tables written through it reach command buffers that get it. */
			D3D12BindlessHeap* GetCurrentResourceHeap() const;

			/**
			 * Creates a shader-visible CBV/SRV/UAV heap with a bindless region of @p bindlessCapacity descriptors, followed by the table
			 * region. Returns null on failure.
			 */
			D3D12BindlessHeap* CreateResourceHeap(u32 bindlessCapacity);

			/**
			 * Returns the mutex that guards table writes into the shader-visible CBV/SRV/UAV heaps. Switching the current heap must hold it
			 * from taking the new heap until CopyDirtyTables() completes.
			 */
			Mutex& GetResourceTableMutex() { return mTableRegions[(u32)D3D12DescriptorHeapType::CBV_SRV_UAV].RegionMutex; }

			/** Copies the live tables @p heap lacks from the table mirror into it, after it became current. Must be called with GetResourceTableMutex() held. */
			void CopyDirtyTables(D3D12BindlessHeap& heap);

			/**
			 * Sets the pool of the shader-visible CBV/SRV/UAV heaps of a device with bindless support, which D3D12BindlessHeaps owns. Must
			 * be set before any table is written or command buffer records, and cleared before the pool is destroyed.
			 */
			void SetResourceHeapPool(GpuBindlessHeapPool* pool) { mResourceHeapPool = pool; }

			/** Destroys the CBV/SRV/UAV heap of a device without bindless support. Must be called before the resource manager is destroyed, once no command buffer binds it. */
			void DestroyFallbackResourceHeap();

			/** Writes @p source into slot @p slotIndex of the bindless region of the shader-visible sampler heap. Nothing may read the slot. */
			void WriteBindlessSamplerSlot(u32 slotIndex, D3D12_CPU_DESCRIPTOR_HANDLE source);

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

			/** Number of descriptors in the table region of every shader-visible CBV/SRV/UAV heap. */
			static constexpr u32 kResourceTableDescriptorCount = 64 * 1024;

			/** Minimum number of descriptors in the table region of the shader-visible sampler heap. The bindless region takes the rest. */
			static constexpr u32 kSamplerTableDescriptorCount = 1024;

		private:
			/** CPU-only heap that individual descriptors are allocated from. */
			struct StagingAllocator
			{
				D3D12DescriptorHeap Heap;
				u32 NextFreeIndex = 0;		/**< Bump-allocation cursor into the heap. */
				Vector<u32> FreeList;		/**< Indices returned by FreeCPUDescriptor(), reused before the cursor advances. */
				Mutex AllocationMutex;		/**< Guards descriptor allocation and release. */
			};

			/** Allocation of the table region of the shader-visible heaps of one type. Offsets are relative to the start of the table region. */
			struct TableRegion
			{
				RangeAllocator FreeRanges;
				TBitfield<> LiveDescriptors;	/**< Descriptors of allocated tables. */

				/** Guards the allocation. For the CBV/SRV/UAV region, also guards table writes and heap switches. */
				Mutex RegionMutex;
			};

			/** Creates the shader-visible sampler heap and the CPU-only heaps. */
			void CreateHeaps(u32 bindlessSamplerCount);

			/** Creates the null CBV/SRV/UAV descriptors unset resource bindings fall back to. */
			void CreateNullDescriptors();

			/** Number of D3D12DescriptorHeapType values. */
			static constexpr u32 kHeapTypeCount = 4;

			/** Number of heap types with shader-visible tables. Relies on CBV_SRV_UAV and Sampler being the first two D3D12DescriptorHeapType values. */
			static constexpr u32 kTableHeapTypeCount = 2;

			D3D12GpuDevice& mDevice;
			StagingAllocator mStagingAllocators[kHeapTypeCount];	/**< CPU-only heaps of every type, indexed by D3D12DescriptorHeapType. */
			TableRegion mTableRegions[kTableHeapTypeCount];			/**< Table region allocation of the shader-visible heaps, indexed by D3D12DescriptorHeapType. */
			D3D12DescriptorHeap mSamplerHeap;						/**< Shader-visible sampler heap. */
			TUnique<GpuBindlessHeapPool> mFallbackResourceHeapPool;	/**< Holds the only CBV/SRV/UAV heap of a device without bindless support, which never switches. Null otherwise. */
			GpuBindlessHeapPool* mResourceHeapPool = nullptr;		/**< Pool of the shader-visible CBV/SRV/UAV heaps: the fallback, or the one the bindless heaps own. */
			D3D12DescriptorHeap mTableMirror;						/**< CPU-only copy of the CBV/SRV/UAV table region, holding the latest descriptors of every table. */

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
