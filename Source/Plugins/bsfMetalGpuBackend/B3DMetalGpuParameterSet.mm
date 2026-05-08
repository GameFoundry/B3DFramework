//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuParameterSet.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "B3DMetalGpuParameterSetPool.h"
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalTexture.h"
#include "B3DMetalSamplerState.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		struct MetalGpuParameters::Impl
		{
			id<MTLBuffer> ArgumentBuffer = nil;
			// Per-instance argument encoder. MTLArgumentEncoder carries per-instance state (the
			// currently bound argument buffer + offset) and is not safe to share across parameter
			// sets, so every MetalGpuParameters mints its own from the layout's descriptor list.
			id<MTLArgumentEncoder> Encoder = nil;

			// B9: byte offset into @c ArgumentBuffer where this set's slice begins. Zero when the
			// buffer was allocated directly (non-pooled path) or for the first slice in a block.
			// Every BindEncoder call passes this as @c setArgumentBuffer:offset: so slice-boundary
			// writes land at the right place within the shared block.
			u64 ArgumentBufferOffset = 0;
		};

		namespace
		{
			template <typename BindingT>
			BindingT* FindBinding(Vector<BindingT>& bindings, u32 slot, u32 arrayIndex)
			{
				for (auto& binding : bindings)
				{
					if (binding.Slot == slot && binding.ArrayIndex == arrayIndex)
						return &binding;
				}

				return nullptr;
			}

			/**
			 * Packs a @c TextureSurface into a 32-bit signature for the dirty-compare snapshot. Only
			 * distinguishes enough of the surface to detect a genuine change; we do not need to reconstruct
			 * the surface from the packed value.
			 */
			u32 PackSurfaceSignature(const TextureSurface& surface)
			{
				return ((u32)(surface.MipLevel & 0xFFu) << 0)
					| ((u32)(surface.MipLevelCount & 0xFFu) << 8)
					| ((u32)(surface.Face & 0xFFu) << 16)
					| ((u32)(surface.FaceCount & 0xFFu) << 24);
			}
		} // namespace

		MetalGpuParameters::MetalGpuParameters(MetalGpuDevice& gpuDevice, const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex, MetalGpuParameterSetPool* pool)
			: GpuParameterSet(parameterSetLayout, setIndex)
			, mGpuDevice(gpuDevice)
			, mImpl(B3DMakeUnique<Impl>())
			, mPool(pool)
		{
			mMetalLayout = static_cast<const MetalGpuPipelineParameterSetLayout*>(parameterSetLayout.get());
		}

		MetalGpuParameters::~MetalGpuParameters()
		{
			if (mImpl)
			{
				mImpl->ArgumentBuffer = nil;
				mImpl->Encoder = nil;
			}
		}

		void MetalGpuParameters::Initialize()
		{
			if (mMetalLayout != nullptr)
			{
				const u64 bufferSize = mMetalLayout->GetArgumentBufferSize();
				if (bufferSize != 0)
				{
					// B9: when a pool is attached, sub-allocate the argument-buffer slice from it so
					// Reset() genuinely recycles memory instead of churning a fresh MTLBuffer per set.
					// A nullptr pool means the set was constructed outside the pool path (tests,
					// persistent-standalone sets) — fall back to a direct device allocation with the
					// same storage mode the pool would have used.
					if (mPool != nullptr)
					{
						const u32 alignment = std::max<u32>(1u, mMetalLayout->GetArgumentBufferAlignment());
						u64 offset = 0;
						mImpl->ArgumentBuffer = mPool->AcquireArgumentBufferSlice(bufferSize, alignment, offset);
						mImpl->ArgumentBufferOffset = offset;
					}
					else
					{
						id<MTLDevice> device = mGpuDevice.GetMetalDevice();
						// Shared storage keeps the argument buffer addressable from the CPU without
						// explicit sync; argument buffers on every supported GPU family can be read from
						// any stage without requiring a managed storage mode.
						mImpl->ArgumentBuffer = [device newBufferWithLength:(NSUInteger)bufferSize
																	options:MTLResourceStorageModeShared];
						mImpl->ArgumentBufferOffset = 0;
					}

					if (mImpl->ArgumentBuffer == nil)
					{
						B3D_LOG(Error, LogRenderBackend,
							"Failed to allocate a {0}-byte Metal argument buffer for parameter set {1}.",
							bufferSize, GetSet());
					}
					else
					{
						// Each parameter set owns its own encoder. MTLArgumentEncoder holds the last
						// setArgumentBuffer call's state, so sharing one across sets would serialize all
						// SetX calls on that layout; minting per-instance keeps updates to disjoint sets
						// independent (and thread-safe).
						mImpl->Encoder = mMetalLayout->NewArgumentEncoder();
					}
				}

				// B6: size the resolved-resource cache to one slot per layout binding. The cache is
				// sparse — entries stay nullptr until CommitPendingBindings drains a dirty slot and
				// stores the resolved id<MTLResource> at its argIndex — so the vector is cheap to
				// allocate once and never resized later.
				mResolvedResources.assign((size_t)mMetalLayout->GetBindings().Size(), nullptr);
			}

			// The base Initialize() registers this parameter set as a CoreObject and triggers render-proxy
			// creation (the seam that sync's SetX bindings from the sim thread to the render thread).
			// Must run unconditionally — even when the Metal-side allocation above was skipped/failed —
			// so GetUniformBuffer / render-proxy sync reflect the set's logical bindings regardless of
			// the Metal argument buffer's state. Placed at the end to mirror the Vulkan backend so the
			// render proxy only fires after all backend-specific state is initialized.
			GpuParameterSet::Initialize();
		}

		id<MTLBuffer> MetalGpuParameters::GetArgumentBuffer() const
		{
			return mImpl ? mImpl->ArgumentBuffer : nil;
		}

		u64 MetalGpuParameters::GetArgumentBufferOffset() const
		{
			return mImpl ? mImpl->ArgumentBufferOffset : 0;
		}

		void MetalGpuParameters::BindEncoder() const
		{
			if (!mImpl || mImpl->ArgumentBuffer == nil || mImpl->Encoder == nil)
				return;

			// B9: offset is the slice origin within the shared pool block. Always zero for non-pooled
			// allocations; the pool's AcquireArgumentBufferSlice returns a per-set offset otherwise.
			[mImpl->Encoder setArgumentBuffer:mImpl->ArgumentBuffer offset:(NSUInteger)mImpl->ArgumentBufferOffset];
		}

		bool MetalGpuParameters::SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex, u32 offset)
		{
			@autoreleasepool
			{
			Lock lock(mSetMutex);

			// Delegate to the base first so mUniformBufferData[slot] is populated — without this,
			// GetUniformBuffer(slot) returns null and the render-proxy sync packet is incomplete. Base
			// rejects out-of-range slots with a Warning; propagate the rejection so we don't grow the
			// Metal-side mirror for a slot the layout doesn't know about.
			if (!GpuParameterSet::SetUniformBuffer(slot, uniformBuffer, arrayIndex, offset))
				return false;

			if (auto* existing = FindBinding(mUniformBuffers, slot, arrayIndex))
			{
				existing->Buffer = uniformBuffer;
				existing->Offset = offset;
			}
			else
			{
				UniformBufferBinding binding;
				binding.Slot = slot;
				binding.ArrayIndex = arrayIndex;
				binding.Offset = offset;
				binding.Buffer = uniformBuffer;
				mUniformBuffers.push_back(std::move(binding));
			}

			if (mMetalLayout != nullptr)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::UniformBuffer, slot);
				if (argIndex != (u32)~0u)
				{
					// B4 / A'5: dirty-compare on (engine-side TShared target, Metal handle, array index,
					// offset). A set-and-forget binding (same buffer, same offset every frame) no-ops
					// after its first SetUniformBuffer. Metal-handle compare catches recreate /
					// address-recycle cases where the TShared target is stable but the backing MTLBuffer
					// changed (or vice versa).
					ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
					const void* incoming = uniformBuffer.get();
					auto mtlWrapper = std::static_pointer_cast<MetalGpuBuffer>(uniformBuffer);
					id<MTLBuffer> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalBuffer() : nil;
					void* incomingHandlePtr = (__bridge void*)incomingHandle;
					if (snapshot.Type != GpuParameterType::UniformBuffer
						|| snapshot.Resource != incoming
						|| snapshot.MetalHandle != incomingHandlePtr
						|| snapshot.ArrayIndex != arrayIndex
						|| snapshot.Offset != offset)
					{
						snapshot.Type = GpuParameterType::UniformBuffer;
						snapshot.Resource = incoming;
						snapshot.MetalHandle = incomingHandlePtr;
						snapshot.ArrayIndex = arrayIndex;
						snapshot.Offset = offset;
						snapshot.Size = 0;
						mDirtyArgumentSlots.insert(argIndex);
						// B3: a genuine binding change — bump the generation so the command-buffer
						// residency-elision cache observes a fresh value and re-emits @c useResources:
						// on the next bind. Bumped here (not per-Set call) because the B4 compare
						// already isolates "actually changed" from "re-set same value".
						++mGeneration;
					}
				}
			}

			return true;
			} // @autoreleasepool
		}

		bool MetalGpuParameters::SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
		{
			@autoreleasepool
			{
			Lock lock(mSetMutex);

			// Base writes mSampledTextureData[slot] and flags the render-proxy sync dirty bit; without
			// this call SetParameter<Texture>(name, value) looks successful but never reaches the GPU.
			if (!GpuParameterSet::SetSampledTexture(slot, texture, surface, arrayIndex))
				return false;

			if (auto* existing = FindBinding(mSampledTextures, slot, arrayIndex))
			{
				existing->Texture = texture;
				existing->Surface = surface;
			}
			else
			{
				TextureBinding binding;
				binding.Slot = slot;
				binding.ArrayIndex = arrayIndex;
				binding.Texture = texture;
				binding.Surface = surface;
				mSampledTextures.push_back(std::move(binding));
			}

			if (mMetalLayout != nullptr)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::SampledTexture, slot);
				if (argIndex != (u32)~0u)
				{
					// B4 / A'5: dirty-compare on (texture pointer, Metal handle, array index, packed
					// surface signature). Metal-handle compare catches @c RecreateInternalTexture —
					// the engine-side @c Texture TShared is stable but the backing MTLTexture changed.
					ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
					const void* incoming = texture.get();
					auto mtlWrapper = std::static_pointer_cast<MetalTexture>(texture);
					id<MTLTexture> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalTexture() : nil;
					void* incomingHandlePtr = (__bridge void*)incomingHandle;
					const u32 surfaceSig = PackSurfaceSignature(surface);
					if (snapshot.Type != GpuParameterType::SampledTexture
						|| snapshot.Resource != incoming
						|| snapshot.MetalHandle != incomingHandlePtr
						|| snapshot.ArrayIndex != arrayIndex
						|| snapshot.Size != surfaceSig)
					{
						snapshot.Type = GpuParameterType::SampledTexture;
						snapshot.Resource = incoming;
						snapshot.MetalHandle = incomingHandlePtr;
						snapshot.ArrayIndex = arrayIndex;
						snapshot.Offset = 0;
						snapshot.Size = surfaceSig;
						mDirtyArgumentSlots.insert(argIndex);
						++mGeneration; // B3
					}
				}
			}

			return true;
			} // @autoreleasepool
		}

		bool MetalGpuParameters::SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
		{
			@autoreleasepool
			{
			Lock lock(mSetMutex);

			// Base writes mStorageTextureData[slot]; without this, GetStorageTexture / render-proxy sync
			// silently drop the binding.
			if (!GpuParameterSet::SetStorageTexture(slot, texture, surface, arrayIndex))
				return false;

			if (auto* existing = FindBinding(mStorageTextures, slot, arrayIndex))
			{
				existing->Texture = texture;
				existing->Surface = surface;
			}
			else
			{
				TextureBinding binding;
				binding.Slot = slot;
				binding.ArrayIndex = arrayIndex;
				binding.Texture = texture;
				binding.Surface = surface;
				mStorageTextures.push_back(std::move(binding));
			}

			if (mMetalLayout != nullptr)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageTexture, slot);
				if (argIndex != (u32)~0u)
				{
					// B4 / A'5: dirty-compare on (texture pointer, Metal handle, array index, packed
					// surface signature). Metal-handle compare catches @c RecreateInternalTexture —
					// stable engine-side TShared, swapped MTLTexture.
					ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
					const void* incoming = texture.get();
					auto mtlWrapper = std::static_pointer_cast<MetalTexture>(texture);
					id<MTLTexture> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalTexture() : nil;
					void* incomingHandlePtr = (__bridge void*)incomingHandle;
					const u32 surfaceSig = PackSurfaceSignature(surface);
					if (snapshot.Type != GpuParameterType::StorageTexture
						|| snapshot.Resource != incoming
						|| snapshot.MetalHandle != incomingHandlePtr
						|| snapshot.ArrayIndex != arrayIndex
						|| snapshot.Size != surfaceSig)
					{
						snapshot.Type = GpuParameterType::StorageTexture;
						snapshot.Resource = incoming;
						snapshot.MetalHandle = incomingHandlePtr;
						snapshot.ArrayIndex = arrayIndex;
						snapshot.Offset = 0;
						snapshot.Size = surfaceSig;
						mDirtyArgumentSlots.insert(argIndex);
						++mGeneration; // B3
					}
				}
			}

			return true;
			} // @autoreleasepool
		}

		bool MetalGpuParameters::SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex, GpuBufferViewInformation view)
		{
			@autoreleasepool
			{
			Lock lock(mSetMutex);

			// Base writes mStorageBufferData[slot]; without this, GetStorageBuffer / render-proxy sync
			// silently drop the binding.
			if (!GpuParameterSet::SetStorageBuffer(slot, buffer, arrayIndex, view))
				return false;

			if (auto* existing = FindBinding(mStorageBuffers, slot, arrayIndex))
			{
				existing->Buffer = buffer;
				existing->View = view;
			}
			else
			{
				StorageBufferBinding binding;
				binding.Slot = slot;
				binding.ArrayIndex = arrayIndex;
				binding.Buffer = buffer;
				binding.View = view;
				mStorageBuffers.push_back(std::move(binding));
			}

			if (mMetalLayout != nullptr)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageBuffer, slot);
				if (argIndex != (u32)~0u)
				{
					// B4 / A'5: dirty-compare on (buffer pointer, Metal handle, array index, view
					// offset, view range). Metal-handle compare catches @c RecreateInternalBuffer —
					// stable engine-side TShared, swapped MTLBuffer.
					ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
					const void* incoming = buffer.get();
					auto mtlWrapper = std::static_pointer_cast<MetalGpuBuffer>(buffer);
					id<MTLBuffer> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalBuffer() : nil;
					void* incomingHandlePtr = (__bridge void*)incomingHandle;
					if (snapshot.Type != GpuParameterType::StorageBuffer
						|| snapshot.Resource != incoming
						|| snapshot.MetalHandle != incomingHandlePtr
						|| snapshot.ArrayIndex != arrayIndex
						|| snapshot.Offset != view.Offset
						|| snapshot.Size != view.Range)
					{
						snapshot.Type = GpuParameterType::StorageBuffer;
						snapshot.Resource = incoming;
						snapshot.MetalHandle = incomingHandlePtr;
						snapshot.ArrayIndex = arrayIndex;
						snapshot.Offset = view.Offset;
						snapshot.Size = view.Range;
						mDirtyArgumentSlots.insert(argIndex);
						++mGeneration; // B3
					}
				}
			}

			return true;
			} // @autoreleasepool
		}

		bool MetalGpuParameters::SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex)
		{
			@autoreleasepool
			{
			Lock lock(mSetMutex);

			// Base writes mSamplerStates[slot]; without this, GetSamplerState / render-proxy sync silently
			// drop the binding.
			if (!GpuParameterSet::SetSamplerState(slot, sampler, arrayIndex))
				return false;

			if (auto* existing = FindBinding(mSamplers, slot, arrayIndex))
			{
				existing->Sampler = sampler;
			}
			else
			{
				SamplerBinding binding;
				binding.Slot = slot;
				binding.ArrayIndex = arrayIndex;
				binding.Sampler = sampler;
				mSamplers.push_back(std::move(binding));
			}

			if (mMetalLayout != nullptr)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::Sampler, slot);
				if (argIndex != (u32)~0u)
				{
					// B4 / A'5: dirty-compare on (sampler pointer, Metal handle, array index).
					// MTLSamplerState is immutable post-creation, but the engine-side TShared could be
					// freshly-constructed while the prior TShared target address is recycled — the
					// handle compare still guards that case.
					ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
					const void* incoming = sampler.get();
					auto mtlWrapper = std::static_pointer_cast<MetalSamplerState>(sampler);
					id<MTLSamplerState> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalSampler() : nil;
					void* incomingHandlePtr = (__bridge void*)incomingHandle;
					if (snapshot.Type != GpuParameterType::Sampler
						|| snapshot.Resource != incoming
						|| snapshot.MetalHandle != incomingHandlePtr
						|| snapshot.ArrayIndex != arrayIndex)
					{
						snapshot.Type = GpuParameterType::Sampler;
						snapshot.Resource = incoming;
						snapshot.MetalHandle = incomingHandlePtr;
						snapshot.ArrayIndex = arrayIndex;
						snapshot.Offset = 0;
						snapshot.Size = 0;
						mDirtyArgumentSlots.insert(argIndex);
					}
				}
			}

			return true;
			} // @autoreleasepool
		}

		bool MetalGpuParameters::SetDynamicOffset(u32 slot, u32 offset)
		{
			@autoreleasepool
			{
			Lock lock(mSetMutex);

			// Find the currently-bound buffer for this slot; we need it to flow the offset change through
			// the base's SetUniformBuffer so mUniformBufferData[slot].Offset stays coherent. Without this
			// re-route the render-proxy sync packet would emit the stale offset and GetUniformBuffer's
			// .Offset field would drift from the Metal-side mirror.
			UniformBufferBinding* found = nullptr;
			for (auto& binding : mUniformBuffers)
			{
				if (binding.Slot == slot)
				{
					found = &binding;
					break;
				}
			}

			if (!found)
				return false;

			// Delegate to base with the existing buffer + arrayIndex and the new offset. Base rejects
			// unknown slots with a Warning; propagate that.
			if (!GpuParameterSet::SetUniformBuffer(slot, found->Buffer, found->ArrayIndex, offset))
				return false;

			found->Offset = offset;

			// There is no encoder-level @c setVertexBufferOffset equivalent when the binding lives inside
			// an argument buffer; the only way to update the per-slot offset is to re-run the argument
			// encoder. Defer that to @c CommitPendingBindings so repeated offset flips on the same slot
			// between draws collapse into a single encoder write.
			//
			// TODO: drop the encoder-rewrite on this path entirely by routing dynamic offsets through
			// a `spvDynamicOffsets [[buffer(N)]]` stage input (SPIRV-Cross `add_dynamic_buffer` +
			// `msl_options.dynamic_offsets_buffer_index`, the same approach MoltenVK uses). That would
			// collapse the cost to a ring-buffer memcpy + one `setVertexBufferOffset:` / `setFragmentBufferOffset:`
			// per draw — roughly 5-10x faster than the argument-encoder path and matching the Vulkan
			// backend's effectively-free dynamic-offset cost. Blocked on adding the MSL pieces to the
			// vendored SPIRV-Cross dependency (not shipped in Framework/Dependencies/SPIRVCross today);
			// re-evaluate once the Metal backend's build prereq is resolved. See S13b in the
			// phase-2 review plan for the full design.
			if (mMetalLayout != nullptr)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::UniformBuffer, slot);
				if (argIndex != (u32)~0u)
				{
					// B4 / A'5: only dirty when the offset truly changed. Repeated
					// SetDynamicBufferOffset calls emitting the same offset between draws become a
					// no-op. Metal-handle is re-resolved from the existing binding — normally
					// unchanged, but a recreate between a @c SetUniformBuffer and this call would
					// swap it without dirtying the B4 @c Resource field alone.
					ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
					const void* incoming = found->Buffer.get();
					auto mtlWrapper = std::static_pointer_cast<MetalGpuBuffer>(found->Buffer);
					id<MTLBuffer> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalBuffer() : nil;
					void* incomingHandlePtr = (__bridge void*)incomingHandle;
					if (snapshot.Type != GpuParameterType::UniformBuffer
						|| snapshot.Resource != incoming
						|| snapshot.MetalHandle != incomingHandlePtr
						|| snapshot.ArrayIndex != found->ArrayIndex
						|| snapshot.Offset != offset)
					{
						snapshot.Type = GpuParameterType::UniformBuffer;
						snapshot.Resource = incoming;
						snapshot.MetalHandle = incomingHandlePtr;
						snapshot.ArrayIndex = found->ArrayIndex;
						snapshot.Offset = offset;
						snapshot.Size = 0;
						mDirtyArgumentSlots.insert(argIndex);
					}
				}
			}

			return true;
			} // @autoreleasepool
		}

		void MetalGpuParameters::CommitPendingBindings()
		{
			Lock lock(mSetMutex);

			if (mDirtyArgumentSlots.empty())
				return;

			if (!mImpl || mImpl->ArgumentBuffer == nil || mImpl->Encoder == nil || mMetalLayout == nullptr)
			{
				mDirtyArgumentSlots.clear();
				return;
			}

			// Argument-encoder calls don't directly autorelease, but inner SetName/debug-label paths on
			// the backing objects do. Under the fiber scheduler this can leak Obj-C temporaries across
			// frames; drain locally.
			@autoreleasepool
			{
			id<MTLArgumentEncoder> encoder = mImpl->Encoder;

			// Bind the argument buffer once up-front: each per-binding encoder call below only writes a
			// slot within it, so we don't need to re-target the encoder per slot.
			BindEncoder();

			// Size-guard the resolved-resource cache in case a parameter set bound bindings before
			// Initialize() completed the allocation. Cheap — no-op once the cache is at full size.
			const u32 layoutBindingCount = (u32)mMetalLayout->GetBindings().Size();
			if ((u32)mResolvedResources.size() < layoutBindingCount)
				mResolvedResources.resize((size_t)layoutBindingCount, nullptr);

			for (const auto& binding : mUniformBuffers)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::UniformBuffer, binding.Slot);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlBuffer = std::static_pointer_cast<MetalGpuBuffer>(binding.Buffer);
				id<MTLBuffer> buffer = mtlBuffer ? mtlBuffer->GetMetalBuffer() : nil;
				[encoder setBuffer:buffer offset:binding.Offset atIndex:argIndex];

				// B6: cache the resolved handle so the useResource: emission path does not re-scan the
				// per-type vectors to find this binding again. Index is the argIndex the layout handed
				// us above.
				if (argIndex < (u32)mResolvedResources.size())
					mResolvedResources[argIndex] = (__bridge void*)buffer;
			}

			for (const auto& binding : mStorageBuffers)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageBuffer, binding.Slot);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlBuffer = std::static_pointer_cast<MetalGpuBuffer>(binding.Buffer);
				id<MTLBuffer> metalBuffer = mtlBuffer ? mtlBuffer->GetMetalBuffer() : nil;
				[encoder setBuffer:metalBuffer offset:0 atIndex:argIndex];

				if (argIndex < (u32)mResolvedResources.size())
					mResolvedResources[argIndex] = (__bridge void*)metalBuffer;
			}

			for (const auto& binding : mSampledTextures)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::SampledTexture, binding.Slot);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlTexture = std::static_pointer_cast<MetalTexture>(binding.Texture);
				id<MTLTexture> tex = mtlTexture ? mtlTexture->GetMetalTexture() : nil;
				[encoder setTexture:tex atIndex:argIndex];

				if (argIndex < (u32)mResolvedResources.size())
					mResolvedResources[argIndex] = (__bridge void*)tex;
			}

			for (const auto& binding : mStorageTextures)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageTexture, binding.Slot);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlTexture = std::static_pointer_cast<MetalTexture>(binding.Texture);
				id<MTLTexture> tex = mtlTexture ? mtlTexture->GetMetalTexture() : nil;
				[encoder setTexture:tex atIndex:argIndex];

				if (argIndex < (u32)mResolvedResources.size())
					mResolvedResources[argIndex] = (__bridge void*)tex;
			}

			for (const auto& binding : mSamplers)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::Sampler, binding.Slot);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlSampler = std::static_pointer_cast<MetalSamplerState>(binding.Sampler);
				id<MTLSamplerState> state = mtlSampler ? mtlSampler->GetMetalSampler() : nil;
				[encoder setSamplerState:state atIndex:argIndex];

				// Samplers never flow into useResource: (they are stage-inherent in Metal), so there is no
				// benefit to caching their handle — leave mResolvedResources[argIndex] as nullptr.
			}

			mDirtyArgumentSlots.clear();
			} // @autoreleasepool
		}

		bool MetalGpuParameters::HasPendingBindings() const
		{
			Lock lock(mSetMutex);
			return !mDirtyArgumentSlots.empty();
		}

		id<MTLResource> MetalGpuParameters::GetCachedResourceForArgIndex(u32 argIndex) const
		{
			Lock lock(mSetMutex);
			if (argIndex >= (u32)mResolvedResources.size())
				return nil;
			return (__bridge id<MTLResource>)mResolvedResources[argIndex];
		}

		u64 MetalGpuParameters::GetGeneration() const
		{
			// B3: synchronized load. The writer paths bump @c mGeneration under @c mSetMutex, so a
			// reader that takes the same lock sees a consistent value; without the lock a u64 load
			// on a non-aligned or tearing architecture could observe a partial write. Cheap — the
			// command-buffer bind path calls this at most a handful of times per draw.
			Lock lock(mSetMutex);
			return mGeneration;
		}
	} // namespace render
} // namespace b3d
