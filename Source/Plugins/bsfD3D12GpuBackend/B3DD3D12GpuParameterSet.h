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

			/**
			 * Registers every resource bound to this set with the command buffer's resource tracker, queuing any
			 * required barriers/transitions into @p barrierHelper (the caller executes them before the work is
			 * recorded). Must run on every bind: each command buffer needs its own tracker registrations.
			 *
			 * @param[in]	resourceTracker	Tracker owned by the command buffer the set is being bound on.
			 * @param[in]	barrierHelper	Barrier helper associated with @p resourceTracker.
			 */
			void TrackBoundResources(D3D12ResourceTracker& resourceTracker, D3D12BarrierHelper& barrierHelper);

			bool SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex = 0) override;

		private:
			/** Information about a bound resource descriptor (a single array element of a binding). */
			struct BoundDescriptor
			{
				D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = { 0 };
				bool IsDirty = true;
			};

			/**
			 * A single descriptor table bound to one root parameter. The root signature emits one root parameter per
			 * binding (see B3DD3D12GpuPipelineParameterLayout::CreateRootSignature), so each used binding maps to exactly
			 * one table containing ArraySize descriptors.
			 */
			struct DescriptorTable
			{
				u32 Slot = 0;
				u32 RootParameterIndex = 0;
				u32 DescriptorCount = 0;

				// CPU descriptors, one per array element of this binding.
				Vector<BoundDescriptor> Descriptors;

				// GPU-visible descriptor range this table is copied into for binding.
				D3D12_CPU_DESCRIPTOR_HANDLE GPUVisibleCPUStart = { 0 };
				D3D12_GPU_DESCRIPTOR_HANDLE GPUVisibleGPUStart = { 0 };

				bool IsDirty = true;
			};

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in the resource tables and flags it dirty. */
			void SetDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in the sampler tables and flags it dirty. */
			void SetSamplerDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Allocates GPU-visible descriptor ranges for all tables. */
			void AllocateGPUDescriptorRanges(D3D12GpuDevice& device);

			/** Copies dirty descriptors from CPU to GPU-visible heap. */
			void UpdateGPUDescriptors(D3D12GpuDevice& device);

			D3D12GpuDevice& mDevice;

			// Resource (CBV/SRV/UAV) descriptor tables, keyed by slot.
			UnorderedMap<u32, DescriptorTable> mDescriptorTables;

			// Sampler descriptor tables (separate GPU-visible heap), keyed by slot.
			UnorderedMap<u32, DescriptorTable> mSamplerTables;

			bool mDescriptorsAllocated = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
