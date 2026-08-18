//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuParameterSet.h"

namespace b3d
{
	namespace render
	{
		class D3D12DescriptorTable;
		struct D3D12DescriptorSetLayout;
		struct D3D12DescriptorTableLayout;

		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of GpuParameterSet. */
		class D3D12GpuParameters : public GpuParameterSet
		{
		public:
			/** Creates a parameter set for @p setIndex of @p parameterSetLayout. */
			D3D12GpuParameters(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, D3D12GpuDevice& device, u32 setIndex);
			~D3D12GpuParameters() override;

			void Initialize() override;

			/**
			 * Prepares packed descriptor tables for binding and sets the root CBVs of bound uniform buffers.
			 *
			 * @param	device				Device to use for descriptor operations.
			 * @param	resourceTracker		Command-buffer tracker that retains the emitted descriptor tables.
			 * @param	commandList			Command list to bind descriptor heaps and tables to.
			 * @param	isGraphics			True if binding for graphics pipeline, false for compute.
			 * @param	descriptorSetLayout	D3D12 root parameters and table packing expected by the active pipeline.
			 * @param	dynamicOffsets		Optional dynamic offset overrides keyed by dynamic-offset index (see
			 *								GpuPipelineParameterSetLayout::GetDynamicOffsetIndex). An override
			 *								replaces the bound buffer's suballocation offset for that binding.
			 */
			void BindDescriptors(D3D12GpuDevice& device, D3D12ResourceTracker& resourceTracker, ID3D12GraphicsCommandList* commandList, bool isGraphics,
				const D3D12DescriptorSetLayout& descriptorSetLayout, const UnorderedMap<u32, u32>* dynamicOffsets = nullptr);

			/**
			 * Registers every resource bound to this set with the command buffer's resource tracker, queuing any
			 * required barriers/transitions into @p barrierHelper (the caller executes them before the work is
			 * recorded). Must run before every draw or dispatch so repeated write accesses are tracked.
			 *
			 * @param	resourceTracker		Tracker owned by the command buffer the set is being bound on.
			 * @param	barrierHelper		Barrier helper associated with @p resourceTracker.
			 * @param	pipelineSetLayout	Active pipeline's layout for this set. Resources are matched against this
			 *								layout by slot and tracked only in the shader stages that consume each binding.
			 */
			void TrackBoundResources(D3D12ResourceTracker& resourceTracker, D3D12BarrierHelper& barrierHelper, const GpuPipelineParameterSetLayout& pipelineSetLayout);

			/**
			 * @name GpuParameterSet Interface
			 * @{
			 */

			bool SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex = 0) override;

			/** @} */

		private:
			/** Information about a bound resource descriptor (a single array element of a binding). */
			struct BoundDescriptor
			{
				D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = { 0 }; /**< CPU descriptor copied into the GPU-visible table. */
			};

			/** CPU descriptors belonging to one shader binding. */
			struct DescriptorBinding
			{
				DescriptorBinding() = default;

				Vector<BoundDescriptor> Descriptors; /**< CPU descriptors, one per array element of this binding. */

				/**
				 * Descriptor copied into GPU-visible slots whose binding is unset: a type-compatible null descriptor, or
				 * the default sampler. Freshly allocated ranges contain stale data, so every slot must be initialized.
				 */
				D3D12_CPU_DESCRIPTOR_HANDLE NullDescriptorHandle = { 0 };
			};

			/** GPU-visible snapshot of one packed descriptor table. */
			struct DescriptorTable
			{
				DescriptorTable() = default;

				u32 DescriptorCount = 0; /**< Number of descriptors in the packed table. */
				D3D12DescriptorTable* Resource = nullptr; /**< Tracked GPU-visible descriptor range used by recorded commands. */
				const D3D12DescriptorTableLayout* Layout = nullptr; /**< Active packing used to build Resource. */
				bool IsDirty = true; /**< True when the GPU-visible table must be refreshed. */
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
			};

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in @p bindings. */
			static bool SetBindingDescriptor(UnorderedMap<u32, DescriptorBinding>& bindings, u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in the resource bindings and flags the packed table dirty. */
			void SetDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle)
			{
				if(SetBindingDescriptor(mDescriptorBindings, slot, arrayIndex, handle))
					mResourceTable.IsDirty = true;
			}

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in the sampler bindings and flags the packed table dirty. */
			void SetSamplerDescriptor(u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle)
			{
				if(SetBindingDescriptor(mSamplerBindings, slot, arrayIndex, handle))
					mSamplerTable.IsDirty = true;
			}

			/**
			 * Copies dirty packed tables into GPU-visible descriptor ranges. Reuses a compatible range when no command
			 * buffer references it; otherwise allocates a new range so recorded work retains its descriptor snapshot.
			 */
			void UpdateGPUDescriptors(D3D12GpuDevice& device, const D3D12DescriptorSetLayout& descriptorSetLayout);

			D3D12GpuDevice& mDevice;

			UnorderedMap<u32, DescriptorBinding> mDescriptorBindings; /**< Resource (CBV/SRV/UAV) descriptor bindings, keyed by slot. */
			UnorderedMap<u32, DescriptorBinding> mSamplerBindings; /**< Sampler descriptor bindings, keyed by slot. */
			UnorderedMap<u32, RootConstantBuffer> mRootConstantBuffers; /**< Uniform buffers bound as root CBV descriptors, keyed by slot. */

			DescriptorTable mResourceTable; /**< Packed CBV/SRV/UAV table. */
			DescriptorTable mSamplerTable; /**< Packed sampler table. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
