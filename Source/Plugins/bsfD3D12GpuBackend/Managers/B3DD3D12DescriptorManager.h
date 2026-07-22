//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"

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

		/**
		 * Manages allocation of descriptor heaps and individual descriptors.
		 *
		 * Resource views (via AllocateCPUDescriptor()) live in CPU-only staging heaps; parameter sets copy them into
		 * contiguous shader-visible ranges (via AllocateGPUDescriptorRange()) before binding. The two must not share a
		 * heap, as shader-visible heaps are CPU-write-only and cannot be a descriptor copy source.
		 */
		class D3D12DescriptorManager
		{
		public:
			D3D12DescriptorManager(D3D12GpuDevice& device);
			~D3D12DescriptorManager();

			/** Allocates a descriptor from the CPU-only staging heap of the specified type. */
			D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPUDescriptor(D3D12DescriptorHeapType type);

			/** Frees a previously allocated descriptor. */
			void FreeCPUDescriptor(D3D12DescriptorHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Allocates a contiguous range of GPU-visible descriptors. */
			void AllocateGPUDescriptorRange(D3D12DescriptorHeapType type, u32 count,
				D3D12_CPU_DESCRIPTOR_HANDLE& outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGPUHandle);

			/** Returns the shader-visible descriptor heap of the specified type. */
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

			/** Returns the descriptor increment size for the specified heap type. */
			u32 GetDescriptorIncrementSize(D3D12DescriptorHeapType type) const { return mDescriptorSizes[(u32)type]; }

			/** Returns the descriptor size for the specified heap type (alias for GetDescriptorIncrementSize). */
			u32 GetDescriptorSize(D3D12DescriptorHeapType type) const { return mDescriptorSizes[(u32)type]; }

		private:
			/** Creates the descriptor heaps. */
			void CreateHeaps();

			/** Creates the null CBV/SRV/UAV descriptors unset resource bindings fall back to. */
			void CreateNullDescriptors();

			/** Descriptor heap for a specific type. */
			struct DescriptorHeap
			{
				ComPtr<ID3D12DescriptorHeap> Heap;
				D3D12_CPU_DESCRIPTOR_HANDLE CPUStart{};
				D3D12_GPU_DESCRIPTOR_HANDLE GPUStart{};
				u32 NumDescriptors = 0;
				u32 NextFreeIndex = 0;
				Vector<u32> FreeList;
			};

			/**
			 * Returns the CPU-only staging heap for the given type. RTV/DSV heaps are CPU-only to begin with, while
			 * CBV_SRV_UAV/Sampler have dedicated staging heaps alongside their shader-visible ones.
			 */
			DescriptorHeap& GetStagingHeap(D3D12DescriptorHeapType type);

			D3D12GpuDevice& mDevice;
			DescriptorHeap mHeaps[4]; // Shader-visible CBV_SRV_UAV/Sampler heaps + CPU-only RTV/DSV heaps
			DescriptorHeap mStagingHeaps[2]; // CPU-only staging heaps for CBV_SRV_UAV and Sampler resource views
			u32 mDescriptorSizes[4] = {}; // Descriptor size for each type
			D3D12_CPU_DESCRIPTOR_HANDLE mDefaultSamplerHandle{}; // Fallback for sampler bindings never set by the caller

			// Fallbacks for resource bindings never set by the caller, indexed by view dimension where applicable
			D3D12_CPU_DESCRIPTOR_HANDLE mNullCBVHandle{};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullSRVHandles[D3D12_SRV_DIMENSION_TEXTURECUBEARRAY + 1] = {};
			D3D12_CPU_DESCRIPTOR_HANDLE mNullUAVHandles[D3D12_UAV_DIMENSION_TEXTURE3D + 1] = {};
		};

		/** @} */
	} // namespace render
} // namespace b3d
