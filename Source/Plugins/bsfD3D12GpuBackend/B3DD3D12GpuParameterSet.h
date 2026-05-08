//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuParameterSet.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of GpuParameterSet. */
		class D3D12GpuParameters : public GpuParameterSet
		{
		public:
			D3D12GpuParameters(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, D3D12GpuDevice& device, u32 setIndex);
			~D3D12GpuParameters() override;

			/** @copydoc GpuParameterSet::Initialize */
			void Initialize() override;

			/**
			 * Prepares descriptor tables for binding. Copies descriptors to GPU-visible heap.
			 *
			 * @param[in]	device			Device to use for descriptor operations.
			 * @param[in]	commandList		Command list to bind descriptor heaps and tables to.
			 * @param[in]	isGraphics		True if binding for graphics pipeline, false for compute.
			 */
			void BindDescriptors(D3D12GpuDevice& device, ID3D12GraphicsCommandList* commandList, bool isGraphics);

		protected:
			/** @copydoc GpuParameterSet::WriteParameters */
			void WriteParameters() override;

		private:
			/** Information about a bound resource descriptor. */
			struct BoundDescriptor
			{
				D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = { 0 };
				bool IsDirty = true;
			};

			/** Information about a descriptor table for a parameter set. */
			struct DescriptorTable
			{
				u32 SetIndex = 0;
				u32 RootParameterIndex = 0;
				u32 DescriptorCount = 0;

				// CPU descriptors for each slot in this set
				Vector<BoundDescriptor> Descriptors;

				// GPU-visible descriptor range
				D3D12_CPU_DESCRIPTOR_HANDLE GPUVisibleCPUStart = { 0 };
				D3D12_GPU_DESCRIPTOR_HANDLE GPUVisibleGPUStart = { 0 };

				bool IsDirty = true;
			};

			/** Updates a descriptor in the table. */
			void SetDescriptor(u32 set, u32 slot, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Allocates GPU-visible descriptor ranges for all tables. */
			void AllocateGPUDescriptorRanges(D3D12GpuDevice& device);

			/** Copies dirty descriptors from CPU to GPU-visible heap. */
			void UpdateGPUDescriptors(D3D12GpuDevice& device);

			D3D12GpuDevice& mDevice;

			// Descriptor tables organized by set index
			UnorderedMap<u32, DescriptorTable> mDescriptorTables;

			// Separate tracking for samplers (different heap type)
			UnorderedMap<u32, DescriptorTable> mSamplerTables;

			bool mDescriptorsAllocated = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
