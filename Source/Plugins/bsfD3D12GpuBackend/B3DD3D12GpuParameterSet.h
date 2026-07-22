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
			 * Prepares descriptor tables for binding. Copies descriptors to GPU-visible heap and sets the root CBVs
			 * of bound uniform buffers.
			 *
			 * Root parameter indices are derived from @p pipelineSetLayout (mirroring the root signature's iteration in
			 * D3D12GpuPipelineParameterLayout::CreateRootSignature), NOT from this set's own layout: the engine allows a
			 * parameter set created from one pipeline to stay bound while drawing with other pipelines whose layouts are
			 * compatible (e.g. NVG switches per-mode material variation pipelines under a single bound set). Resources
			 * are matched by slot - slots encode the HLSL register and register class, so a slot match is a type match.
			 *
			 * @param[in]	device				Device to use for descriptor operations.
			 * @param[in]	commandList			Command list to bind descriptor heaps and tables to.
			 * @param[in]	isGraphics			True if binding for graphics pipeline, false for compute.
			 * @param[in]	rootParameterOffset	Index of the pipeline's first root parameter belonging to this set (root
			 *									parameters are numbered flat across all of the pipeline's sets, see
			 *									D3D12GpuPipelineParameterLayout::GetSetRootParameterOffset()).
			 * @param[in]	pipelineSetLayout	The ACTIVE pipeline's layout for this set index, defining the root
			 *									signature's parameter order.
			 * @param[in]	dynamicOffsets		Optional dynamic offset overrides keyed by dynamic-offset index (see
			 *									GpuPipelineParameterSetLayout::GetDynamicOffsetIndex). An override
			 *									replaces the bound buffer's suballocation offset for that binding.
			 */
			void BindDescriptors(D3D12GpuDevice& device, ID3D12GraphicsCommandList* commandList, bool isGraphics, u32 rootParameterOffset,
				const GpuPipelineParameterSetLayout& pipelineSetLayout, const UnorderedMap<u32, u32>* dynamicOffsets = nullptr);

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
			 * one table containing ArraySize descriptors. The root parameter index the table binds to depends on the
			 * active pipeline's layout and is derived at bind time (see BindDescriptors).
			 */
			struct DescriptorTable
			{
				u32 Slot = 0;
				u32 DescriptorCount = 0;

				// CPU descriptors, one per array element of this binding.
				Vector<BoundDescriptor> Descriptors;

				// GPU-visible descriptor range this table is copied into for binding.
				D3D12_CPU_DESCRIPTOR_HANDLE GPUVisibleCPUStart = { 0 };
				D3D12_GPU_DESCRIPTOR_HANDLE GPUVisibleGPUStart = { 0 };

				// Descriptor copied into GPU-visible heap slots whose binding was never (or null-) set: a null
				// CBV/SRV/UAV matching the binding's declared type, or the default sampler for sampler bindings.
				// Freshly allocated GPU-visible ranges hold stale data, so every slot must be written.
				D3D12_CPU_DESCRIPTOR_HANDLE NullDescriptorHandle = { 0 };

				bool IsDirty = true;
			};

			/**
			 * A single-element uniform buffer bound as a root CBV descriptor rather than through a descriptor table
			 * (mirroring B3DD3D12GpuPipelineParameterLayout::CreateRootSignature). Root CBVs take a raw GPU virtual
			 * address, which is how suballocation offsets and per-draw dynamic offsets are applied. The buffer itself
			 * is resolved from the base class's bound-buffer data, and the root parameter index from the active
			 * pipeline's layout, at bind time.
			 */
			struct RootConstantBuffer
			{
				u32 DataIndex = 0; /**< Index into the base class's uniform buffer data (sequential resource index). */
				u32 DynamicOffsetIndex = ~0u; /**< Index dynamic offset overrides are keyed by, or ~0u if unsupported. */
			};

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in the resource tables and flags it dirty. */
			void SetDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in the sampler tables and flags it dirty. */
			void SetSamplerDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/**
			 * Allocates a fresh GPU-visible descriptor range for every table whose descriptors changed since the last
			 * update and copies the full table into it. Ranges are never updated in place: the GPU consumes them at
			 * execution time, so recorded-but-unexecuted work must keep seeing the descriptors it was recorded with.
			 */
			void UpdateGPUDescriptors(D3D12GpuDevice& device);

			D3D12GpuDevice& mDevice;

			// Resource (CBV/SRV/UAV) descriptor tables, keyed by slot.
			UnorderedMap<u32, DescriptorTable> mDescriptorTables;

			// Uniform buffers bound as root CBV descriptors, keyed by slot.
			UnorderedMap<u32, RootConstantBuffer> mRootConstantBuffers;

			// Sampler descriptor tables (separate GPU-visible heap), keyed by slot.
			UnorderedMap<u32, DescriptorTable> mSamplerTables;
		};

		/** @} */
	} // namespace render
} // namespace b3d
