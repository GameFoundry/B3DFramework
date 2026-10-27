//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuParameterSet.h"

namespace b3d
{
	namespace render
	{
		class GpuResourceTracker;
		class D3D12DescriptorTable;
		class D3D12GpuParameterSetPool;

		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of GpuParameterSet. */
		class D3D12GpuParameters : public GpuParameterSet
		{
		public:
			/** Creates a parameter set for @p setIndex of @p parameterSetLayout, allocated from @p pool. */
			D3D12GpuParameters(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, D3D12GpuDevice& device, u32 setIndex, D3D12GpuParameterSetPool& pool);
			~D3D12GpuParameters() override;

			void Initialize() override;

			/**
			 * Prepares packed descriptor tables for binding and sets the root CBVs of bound uniform buffers. On the thread that owns the set,
			 * flushes the set first (see Flush()), with the packing of the active pipeline.
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
			void BindDescriptors(D3D12GpuDevice& device, GpuResourceTracker& resourceTracker, ID3D12GraphicsCommandList* commandList, bool isGraphics,
				const D3D12DescriptorSetLayout& descriptorSetLayout, const UnorderedMap<u32, u32>* dynamicOffsets = nullptr);

			/**
			 * Tracks the resources consumed by the active pipeline.
			 *
			 * @param	pipelineSetLayout	Active pipeline's layout for this set. Resources are matched against this
			 *								layout by slot.
			 * @param	resourceTracker		Tracker retaining resources and recording their accesses.
			 * @return						False if shader accesses overlap a writable attachment.
			 */
			bool TrackResources(const GpuPipelineParameterSetLayout& pipelineSetLayout, GpuResourceTracker& resourceTracker);

			/**
			 * @name GpuParameterSet Interface
			 * @{
			 */

			bool SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex = 0) override;

			/** Packs the descriptor tables as the set's own layout packs them (see D3D12GpuPipelineParameterSetLayout). Descriptors hold no image layouts, so @p attachments are ignored. */
			void Flush(TArrayView<const GpuRenderPassAttachmentUsage> attachments = {}) override;

			/** @} */

		private:
			/** Information about a bound resource descriptor (a single array element of a binding). */
			struct BoundDescriptor
			{
				D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = { 0 }; /**< CPU descriptor copied into the GPU-visible table. */

				/** Resource generation (see D3D12Texture::GetResourceGeneration()) CPUHandle was resolved at. Only used for textures and buffers. */
				u32 ResourceGeneration = 0;
			};

			/** CPU descriptors belonging to one shader binding. */
			struct DescriptorBinding
			{
				DescriptorBinding() = default;

				Vector<BoundDescriptor> Descriptors; /**< CPU descriptors, one per array element of this binding. */

				GpuParameterType Type = GpuParameterType::Count; /**< Type of the binding. */
				u32 FirstDataIndex = ~0u; /**< Index of the first array element in the base class's resource data (sequential resource index). */
				bool IsReadWrite = false; /**< True for storage buffers bound for writing. */

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

				/**
				 * Returns true if the table is up to date and packed as @p layout packs it. Every pipeline layout packs its own copy of a set's
				 * tables, so packings are compared by content.
				 */
				bool IsCurrent(const D3D12DescriptorTableLayout& layout) const;

				u32 DescriptorCount = 0; /**< Number of descriptors in the packed table. */
				D3D12DescriptorTable* Resource = nullptr; /**< Tracked GPU-visible descriptor range used by recorded commands. */
				Vector<D3D12DescriptorBindingLayout> Bindings; /**< Bindings of the packing used to build Resource. */
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
				u32 DynamicOffsetIndex = ~0u; /**< Dynamic-offset index in this parameter set's own layout. */
			};

			/** CPU descriptor of a texture or buffer, and the resource generation it was resolved at. */
			struct ResolvedDescriptor
			{
				D3D12_CPU_DESCRIPTOR_HANDLE Handle = { 0 }; /**< Null if there is no resource, or the resource has no native resource. */
				u32 ResourceGeneration = 0;
			};

			/** Stores a CPU descriptor at the given (slot, arrayIndex) in @p bindings. */
			static bool SetBindingDescriptor(UnorderedMap<u32, DescriptorBinding>& bindings, u32 slot, u32 arrayIndex, D3D12_CPU_DESCRIPTOR_HANDLE handle);

			/** Returns the resource binding at @p slot, or null if the set has none, such as for a uniform buffer bound as a root CBV. */
			DescriptorBinding* FindDescriptorBinding(u32 slot);

			/** Stores @p descriptor at @p arrayIndex of @p binding, and flags the packed table dirty. Ignores out of range array indices. */
			void StoreDescriptor(DescriptorBinding& binding, u32 arrayIndex, const ResolvedDescriptor& descriptor);

			/** Resolves the constant buffer view of @p buffer at @p offset bytes. @p buffer may be null. */
			static ResolvedDescriptor ResolveCBV(const D3D12GpuBuffer* buffer, u32 offset);

			/** Resolves the shader resource view of @p surface of @p texture. @p texture may be null. */
			static ResolvedDescriptor ResolveSRV(D3D12Texture* texture, const TextureSurface& surface);

			/** Resolves the unordered access view of @p surface of @p texture. @p texture may be null. */
			static ResolvedDescriptor ResolveUAV(D3D12Texture* texture, const TextureSurface& surface);

			/**
			 * Resolves the view of a storage buffer bound at @p slot: its unordered access view if @p isReadWrite, and its shader resource view
			 * otherwise, viewing the elements as @p format. Falls back to the other view if the buffer lacks the requested one. @p buffer may be null.
			 */
			ResolvedDescriptor ResolveStorageBuffer(const D3D12GpuBuffer* buffer, GpuBufferFormat format, bool isReadWrite, u32 slot) const;

			/**
			 * Resolves again the descriptors of textures and buffers that recreated their native resource since they were resolved,
			 * which released the views the descriptors were copied from. If @p canWrite is false, only reports them. Returns true if any
			 * descriptor was stale.
			 */
			bool RefreshDescriptors(bool canWrite);

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
