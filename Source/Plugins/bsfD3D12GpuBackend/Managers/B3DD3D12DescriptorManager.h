//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"

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

		/** Tracked allocation within a shader-visible descriptor heap. */
		class D3D12DescriptorTable : public D3D12Resource
		{
		public:
			D3D12DescriptorTable(D3D12ResourceManager* owner, D3D12DescriptorManager& descriptorManager, D3D12DescriptorHeapType type, u32 startIndex, u32 descriptorCount, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);
			~D3D12DescriptorTable() override;

			/** Returns the first CPU handle in the shader-visible range. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return mCPUHandle; }

			/** Returns the first GPU handle in the shader-visible range. */
			D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return mGPUHandle; }

		private:
			D3D12DescriptorManager& mDescriptorManager;
			D3D12DescriptorHeapType mType;
			u32 mStartIndex;
			u32 mDescriptorCount;
			D3D12_CPU_DESCRIPTOR_HANDLE mCPUHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE mGPUHandle;
		};

		/**
		 * Manages allocation of descriptor heaps and individual descriptors.
		 *
		 * Resource views live in CPU-only staging heaps; parameter sets copy them into tracked, contiguous shader-visible
		 * tables before binding. The two must not share a heap, as shader-visible heaps cannot be a descriptor copy source.
		 *
		 * @note Thread safe.
		 */
		class D3D12DescriptorManager
		{
		public:
			/** Creates the descriptor heaps and immutable fallback descriptors for @p device. */
			D3D12DescriptorManager(D3D12GpuDevice& device);

			/** Allocates a descriptor from the CPU-only staging heap of the specified type. */
			D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPUDescriptor(D3D12DescriptorHeapType type);

			/** Frees a previously allocated descriptor. */
			void FreeCPUDescriptor(D3D12DescriptorHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/**
			 * Allocates a tracked contiguous range of GPU-visible descriptors. Only valid for the CBV_SRV_UAV and Sampler
			 * heap types. Returns null when no free range can satisfy @p count.
			 */
			D3D12DescriptorTable* AllocateGPUDescriptorTable(D3D12DescriptorHeapType type, u32 count);

			/** Returns the primary heap for @p type; CBV/SRV/UAV and sampler heaps are shader-visible. */
			ID3D12DescriptorHeap* GetDescriptorHeap(D3D12DescriptorHeapType type) const;

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
			 * Returns a null SRV descriptor of the given view dimension, used for read-only texture/buffer bindings the
			 * caller never set. Shader reads through it return zero. The dimension must match the shader's declared
			 * resource type for the null-descriptor guarantees to hold.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetNullSRVHandle(D3D12_SRV_DIMENSION dimension) const;

			/**
			 * Returns a null UAV descriptor of the given view dimension, used for read-write texture/buffer bindings
			 * the caller never set. Shader reads through it return zero, writes are dropped. The dimension must match
			 * the shader's declared resource type for the null-descriptor guarantees to hold.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetNullUAVHandle(D3D12_UAV_DIMENSION dimension) const;

			/** Returns the stride between consecutive descriptors in a heap of the specified type. */
			u32 GetDescriptorSize(D3D12DescriptorHeapType type) const { return mDescriptorSizes[(u32)type]; }

		private:
			friend class D3D12DescriptorTable;

			/** Contiguous free range within a shader-visible descriptor heap. */
			struct DescriptorRange
			{
				/** Creates a range starting at @p startIndex and spanning @p descriptorCount descriptors. */
				DescriptorRange(u32 startIndex, u32 descriptorCount) 
					:StartIndex(startIndex), DescriptorCount(descriptorCount) 
				{ }

				u32 StartIndex;      /**< Index of the first descriptor. */
				u32 DescriptorCount; /**< Number of descriptors in the range. */
			};

			/** Descriptor heap for a specific type. */
			struct DescriptorHeap
			{
				ComPtr<ID3D12DescriptorHeap> Heap;		/**< Native descriptor heap. */
				D3D12_CPU_DESCRIPTOR_HANDLE CPUStart{};	/**< CPU handle of the first descriptor. */
				D3D12_GPU_DESCRIPTOR_HANDLE GPUStart{};	/**< Only valid for shader-visible heaps. */
				u32 DescriptorCount = 0;				/**< Capacity the heap was created with. */
				u32 NextFreeIndex = 0;					/**< Bump-allocation cursor into the heap. */
				Vector<u32> FreeList;					/**< Indices returned by FreeCPUDescriptor(), reused before the cursor advances. */
				Vector<DescriptorRange> FreeRanges;		/**< Coalesced free ranges in a shader-visible heap. */
				Mutex AllocationMutex;					/**< Guards descriptor allocation and release. */
			};

			/** Creates the descriptor heaps, both the shader-visible ones and the CPU-only staging heaps. */
			void CreateHeaps();

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
			DescriptorHeap mHeaps[kHeapTypeCount];					/**< Shader-visible CBV_SRV_UAV/Sampler heaps + CPU-only RTV/DSV heaps. */
			DescriptorHeap mStagingHeaps[kStagingHeapTypeCount];	/**< CPU-only staging heaps for CBV_SRV_UAV and Sampler resource views */
			u32 mDescriptorSizes[kHeapTypeCount] = {};				/**< Descriptor size for each type. */

			// Fallbacks for resource bindings never set by the caller, indexed by view dimension where applicable
			D3D12_CPU_DESCRIPTOR_HANDLE mDefaultSamplerHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullCBVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURECUBEARRAY + 1] = {};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE3D + 1] = {};
		};

		/** @} */
	} // namespace render
} // namespace b3d
