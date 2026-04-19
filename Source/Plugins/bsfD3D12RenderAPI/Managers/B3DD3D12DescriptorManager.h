//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
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

		/** Manages allocation of descriptor heaps and individual descriptors. */
		class D3D12DescriptorManager
		{
		public:
			D3D12DescriptorManager(D3D12GpuDevice& device);
			~D3D12DescriptorManager();

			/** Allocates a descriptor from the specified heap type. */
			D3D12_CPU_DESCRIPTOR_HANDLE AllocateCPUDescriptor(D3D12DescriptorHeapType type);

			/** Frees a previously allocated descriptor. */
			void FreeCPUDescriptor(D3D12DescriptorHeapType type, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Allocates a contiguous range of GPU-visible descriptors. */
			void AllocateGPUDescriptorRange(D3D12DescriptorHeapType type, u32 count,
				D3D12_CPU_DESCRIPTOR_HANDLE& outCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGPUHandle);

			/** Returns the descriptor heap of the specified type. */
			ID3D12DescriptorHeap* GetDescriptorHeap(D3D12DescriptorHeapType type) const;

			/** Returns the descriptor increment size for the specified heap type. */
			u32 GetDescriptorIncrementSize(D3D12DescriptorHeapType type) const { return mDescriptorSizes[(u32)type]; }

			/** Returns the descriptor size for the specified heap type (alias for GetDescriptorIncrementSize). */
			u32 GetDescriptorSize(D3D12DescriptorHeapType type) const { return mDescriptorSizes[(u32)type]; }

		private:
			/** Creates the descriptor heaps. */
			void CreateHeaps();

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

			D3D12GpuDevice& mDevice;
			DescriptorHeap mHeaps[4]; // One for each D3D12DescriptorHeapType
			u32 mDescriptorSizes[4] = {}; // Descriptor size for each type
		};

		/** @} */
	} // namespace render
} // namespace b3d
