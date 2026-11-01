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

		/** Native descriptor heap, and the handles of its descriptors. Empty until Create() succeeds. */
		class D3D12DescriptorHeap
		{
		public:
			/**
			 * Creates the native heap. Returns false on failure, and leaves the heap unchanged.
			 *
			 * @param	device				Device to create the heap on.
			 * @param	type				Type of the descriptors in the heap.
			 * @param	capacity			Number of descriptors in the heap.
			 * @param	isShaderVisible		True if shaders read the heap, which also makes GetGPUHandle() valid.
			 * @param	tableRegionStart	First descriptor of the table region of a shader-visible heap. Bindless descriptors precede it.
			 */
			bool Create(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity, bool isShaderVisible, u32 tableRegionStart = 0);

			/** Returns the native heap, or null if the heap was not created. */
			ID3D12DescriptorHeap* GetHeap() const { return mHeap.Get(); }

			/** Returns the type of the descriptors in the heap. */
			D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return mType; }

			/** Returns the number of descriptors in the heap. */
			u32 GetCapacity() const { return mCapacity; }

			/** Returns the stride between consecutive descriptors of the heap. */
			u32 GetDescriptorSize() const { return mDescriptorSize; }

			/** Returns the first descriptor of the table region. Zero for heaps without bindless descriptors. */
			u32 GetTableRegionStart() const { return mTableRegionStart; }

			/** Returns the CPU handle of descriptor @p index. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(u32 index) const
			{
				B3D_ASSERT(index < mCapacity);
				return D3D12_CPU_DESCRIPTOR_HANDLE{ mCPUStart.ptr + (SIZE_T)index * mDescriptorSize };
			}

			/** Returns the GPU handle of descriptor @p index. Only valid for shader-visible heaps. */
			D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(u32 index) const
			{
				B3D_ASSERT(mGPUStart.ptr != 0 && index < mCapacity);
				return D3D12_GPU_DESCRIPTOR_HANDLE{ mGPUStart.ptr + (u64)index * mDescriptorSize };
			}

		private:
			ComPtr<ID3D12DescriptorHeap> mHeap;
			D3D12_DESCRIPTOR_HEAP_TYPE mType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			D3D12_CPU_DESCRIPTOR_HANDLE mCPUStart{};
			D3D12_GPU_DESCRIPTOR_HANDLE mGPUStart{};
			u32 mCapacity = 0;
			u32 mDescriptorSize = 0;
			u32 mTableRegionStart = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
