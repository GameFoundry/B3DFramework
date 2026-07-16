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
			bool OwnsArgumentBuffer = false;

			// B9: byte offset into @c ArgumentBuffer where this set's slice begins. Zero when the
			// buffer was allocated directly (non-pooled path) or for the first slice in a block.
			// Direct writes and command-encoder binds add this to the reflected set-local byte offset.
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

			bool ValidateBufferRange(const TShared<GpuBuffer>& buffer, u32 offset, u32 range, const char* bindingType)
			{
				if (buffer == nullptr)
					return offset == 0 && range == 0;

				const u32 bufferSize = buffer->GetSuballocationSize();
				if ((offset & 15u) != 0 || offset >= bufferSize
					|| (range != 0 && ((u64)offset + range > bufferSize)))
				{
					B3D_LOG(Error, LogRenderBackend,
						"Metal {0} binding has an invalid offset/range. Offset: {1}, range: {2}, buffer size: {3}. "
						"Offsets must be 16-byte aligned.", bindingType, offset, range, bufferSize);
					return false;
				}

				return true;
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
#if !__has_feature(objc_arc)
				if (mImpl->OwnsArgumentBuffer && mImpl->ArgumentBuffer != nil)
					[mImpl->ArgumentBuffer release];
#endif
				mImpl->ArgumentBuffer = nil;
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
						mImpl->OwnsArgumentBuffer = false;
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
						mImpl->OwnsArgumentBuffer = true;
					}

					if (mImpl->ArgumentBuffer == nil)
					{
						B3D_LOG(Error, LogRenderBackend,
							"Failed to allocate a {0}-byte Metal argument buffer for parameter set {1}.",
							bufferSize, GetSet());
					}
					else
					{
						std::memset((u8*)[mImpl->ArgumentBuffer contents] + mImpl->ArgumentBufferOffset, 0, (size_t)bufferSize);
					}
				}

				// Keep one dense cache entry per resource-array element. Entries stay null until
				// CommitPendingBindings drains the corresponding dirty argument ID.
				mResolvedResources.assign((size_t)mMetalLayout->GetResourceCount(), nullptr);
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

		bool MetalGpuParameters::IsMetalBindingReady() const
		{
			if (mMetalLayout == nullptr || mMetalLayout->GetArgumentBufferSize() == 0)
				return true;
			if (mImpl == nullptr || mImpl->ArgumentBuffer == nil)
				return false;
			return true;
		}

		bool MetalGpuParameters::SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex, u32 offset)
		{
			@autoreleasepool
			{
			Lock lock(mSetMutex);
			if (!ValidateBufferRange(uniformBuffer, offset, 0, "uniform-buffer"))
				return false;

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
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::UniformBuffer, slot, arrayIndex);
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
			if (surface.MipLevel != 0 || surface.MipLevelCount != 0 || surface.Face != 0 || surface.FaceCount != 0)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Metal texture subresource bindings require a native texture view and are not supported yet. Slot: {0}.",
					slot);
				return false;
			}

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
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::SampledTexture, slot, arrayIndex);
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
			if (surface.MipLevel != 0 || surface.MipLevelCount != 0 || surface.Face != 0 || surface.FaceCount != 0)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Metal texture subresource bindings require a native texture view and are not supported yet. Slot: {0}.",
					slot);
				return false;
			}

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
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageTexture, slot, arrayIndex);
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

			GpuParameterObjectType objectType = GPOT_UNKNOWN;
			for (const MetalArgumentBufferBinding& binding : mMetalLayout->GetBindings())
			{
				if (binding.Type == GpuParameterType::StorageBuffer && binding.Slot == slot)
				{
					objectType = binding.ObjectType;
					break;
				}
			}
			if (objectType != GPOT_STRUCTURED_BUFFER && objectType != GPOT_RWSTRUCTURED_BUFFER)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Metal typed/raw storage-buffer bindings require a texture-buffer view and are not supported yet. Slot: {0}.",
					slot);
				return false;
			}
			if (!ValidateBufferRange(buffer, view.Offset, view.Range, "storage-buffer"))
				return false;

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
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageBuffer, slot, arrayIndex);
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
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::Sampler, slot, arrayIndex);
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

		bool MetalGpuParameters::SetDynamicOffset(u32 dynamicOffsetIndex, u32 offset)
		{
			@autoreleasepool
			{
			if (mMetalLayout == nullptr)
				return false;

			GpuParameterType bindingType = GpuParameterType::Unknown;
			u32 slot = 0;
			u32 arrayIndex = 0;
			if (!mMetalLayout->GetDynamicOffsetBinding(dynamicOffsetIndex, bindingType, slot, arrayIndex))
				return false;

			Lock lock(mSetMutex);

			// Uniform-buffer path. Find the currently-bound buffer for this slot; we need it to flow
			// the offset change through the base's SetUniformBuffer so mUniformBufferData[slot].Offset
			// stays coherent. Without this re-route the render-proxy sync packet would emit the stale
			// offset and GetUniformBuffer's .Offset field would drift from the Metal-side mirror.
			UniformBufferBinding* foundUniform = nullptr;
			for (auto& binding : mUniformBuffers)
			{
				if (binding.Slot == slot && binding.ArrayIndex == arrayIndex)
				{
					foundUniform = &binding;
					break;
				}
			}

			if (bindingType == GpuParameterType::UniformBuffer && foundUniform != nullptr)
			{
				if (!ValidateBufferRange(foundUniform->Buffer, offset, 0, "uniform-buffer"))
					return false;

				// Delegate to base with the existing buffer + arrayIndex and the new offset. Base
				// rejects unknown slots with a Warning; propagate that.
				if (!GpuParameterSet::SetUniformBuffer(slot, foundUniform->Buffer, foundUniform->ArrayIndex, offset))
					return false;

				foundUniform->Offset = offset;

				// The offset is part of the 64-bit GPU address stored in the argument buffer. Defer the direct
				// rewrite to CommitPendingBindings so repeated changes before a draw collapse to one write.
				if (mMetalLayout != nullptr)
				{
					const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::UniformBuffer, slot, foundUniform->ArrayIndex);
					if (argIndex != (u32)~0u)
					{
						// B4 / A'5: only dirty when the offset truly changed. Repeated
						// SetDynamicBufferOffset calls emitting the same offset between draws become a
						// no-op. Metal-handle is re-resolved from the existing binding — normally
						// unchanged, but a recreate between a @c SetUniformBuffer and this call would
						// swap it without dirtying the B4 @c Resource field alone.
						ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
						const void* incoming = foundUniform->Buffer.get();
						auto mtlWrapper = std::static_pointer_cast<MetalGpuBuffer>(foundUniform->Buffer);
						id<MTLBuffer> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalBuffer() : nil;
						void* incomingHandlePtr = (__bridge void*)incomingHandle;
						if (snapshot.Type != GpuParameterType::UniformBuffer
							|| snapshot.Resource != incoming
							|| snapshot.MetalHandle != incomingHandlePtr
							|| snapshot.ArrayIndex != foundUniform->ArrayIndex
							|| snapshot.Offset != offset)
						{
							snapshot.Type = GpuParameterType::UniformBuffer;
							snapshot.Resource = incoming;
							snapshot.MetalHandle = incomingHandlePtr;
							snapshot.ArrayIndex = foundUniform->ArrayIndex;
							snapshot.Offset = offset;
							snapshot.Size = 0;
							mDirtyArgumentSlots.insert(argIndex);
						}
					}
				}

				return true;
			}

			// Storage-buffer path. The core layout also assigns dynamic-offset indices to structured
			// storage buffers (GpuPipelineParameterSetLayout gives GPOT_STRUCTURED_BUFFER /
			// GPOT_RWSTRUCTURED_BUFFER a DynamicOffsetIndex), so a dynamic offset arriving for a slot
			// bound as a storage buffer must update the view offset included in the direct GPU-address write.
			// Mirrors the Vulkan backend, which folds dynamic offsets into descriptor binds for both
			// buffer kinds.
			StorageBufferBinding* foundStorage = nullptr;
			for (auto& binding : mStorageBuffers)
			{
				if (binding.Slot == slot && binding.ArrayIndex == arrayIndex)
				{
					foundStorage = &binding;
					break;
				}
			}

			if (bindingType != GpuParameterType::StorageBuffer || foundStorage == nullptr)
				return false;

			GpuBufferViewInformation view = foundStorage->View;
			view.Offset = offset;
			if (!ValidateBufferRange(foundStorage->Buffer, view.Offset, view.Range, "storage-buffer"))
				return false;

			// Route through the base so mStorageBufferData[slot].View stays coherent with the
			// Metal-side mirror and the render-proxy sync packet. Base rejects unknown slots with a
			// Warning; propagate that.
			if (!GpuParameterSet::SetStorageBuffer(slot, foundStorage->Buffer, foundStorage->ArrayIndex, view))
				return false;

			foundStorage->View = view;

			if (mMetalLayout != nullptr)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageBuffer, slot, foundStorage->ArrayIndex);
				if (argIndex != (u32)~0u)
				{
					// B4 / A'5: only dirty when the offset truly changed — same rationale as the
					// uniform path. No generation bump: dynamic offsets change the encoded pointer,
					// not the resident resource set.
					ArgumentSlotSnapshot& snapshot = mSlotSnapshots[argIndex];
					const void* incoming = foundStorage->Buffer.get();
					auto mtlWrapper = std::static_pointer_cast<MetalGpuBuffer>(foundStorage->Buffer);
					id<MTLBuffer> incomingHandle = mtlWrapper ? mtlWrapper->GetMetalBuffer() : nil;
					void* incomingHandlePtr = (__bridge void*)incomingHandle;
					if (snapshot.Type != GpuParameterType::StorageBuffer
						|| snapshot.Resource != incoming
						|| snapshot.MetalHandle != incomingHandlePtr
						|| snapshot.ArrayIndex != foundStorage->ArrayIndex
						|| snapshot.Offset != view.Offset
						|| snapshot.Size != view.Range)
					{
						snapshot.Type = GpuParameterType::StorageBuffer;
						snapshot.Resource = incoming;
						snapshot.MetalHandle = incomingHandlePtr;
						snapshot.ArrayIndex = foundStorage->ArrayIndex;
						snapshot.Offset = view.Offset;
						snapshot.Size = view.Range;
						mDirtyArgumentSlots.insert(argIndex);
					}
				}
			}

			return true;
			} // @autoreleasepool
		}

		u64 MetalGpuParameters::CommitPendingBindings()
		{
			Lock lock(mSetMutex);

			if (mDirtyArgumentSlots.empty())
				return mGeneration;

			if (!mImpl || mImpl->ArgumentBuffer == nil || mMetalLayout == nullptr || !IsMetalBindingReady())
			{
				mDirtyArgumentSlots.clear();
				return mGeneration;
			}

			// Resource wrapper access can create Obj-C temporaries under the fiber scheduler; drain locally.
			@autoreleasepool
			{
			static_assert(sizeof(MTLResourceID) == sizeof(u64),
				"Tier-2 argument-buffer resource IDs must be 64-bit values.");
			u8* argumentBytes = (u8*)[mImpl->ArgumentBuffer contents] + GetArgumentBufferOffset();
			const auto fnWriteArgument = [&](GpuParameterType type, u32 slot, u32 arrayIndex,
				const void* value, size_t valueSize)
			{
				const u64 byteOffset = mMetalLayout->GetArgumentBufferByteOffset(type, slot, arrayIndex);
				if(byteOffset == ~0ull || byteOffset + valueSize > mMetalLayout->GetArgumentBufferSize())
				{
					B3D_LOG(Error, LogRenderBackend, "Metal argument-buffer write is outside the reflected layout. "
						"Set: {0}, slot: {1}, type: {2}.", GetSet(), slot, (u32)type);
					return;
				}

				std::memcpy(argumentBytes + byteOffset, value, valueSize);
			};

			// Size-guard the resolved-resource cache in case a parameter set bound bindings before
			// Initialize() completed the allocation. Cheap — no-op once the cache is at full size.
			const u32 layoutResourceCount = mMetalLayout->GetResourceCount();
			if ((u32)mResolvedResources.size() < layoutResourceCount)
				mResolvedResources.resize((size_t)layoutResourceCount, nullptr);

			for (const auto& binding : mUniformBuffers)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::UniformBuffer, binding.Slot, binding.ArrayIndex);
				const u32 resourceIndex = mMetalLayout->GetResourceIndex(GpuParameterType::UniformBuffer, binding.Slot, binding.ArrayIndex);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlBuffer = std::static_pointer_cast<MetalGpuBuffer>(binding.Buffer);
				id<MTLBuffer> buffer = mtlBuffer ? mtlBuffer->GetMetalBuffer() : nil;
				u64 gpuAddress = buffer != nil ? (u64)buffer.gpuAddress + binding.Offset : 0;
				fnWriteArgument(GpuParameterType::UniformBuffer, binding.Slot, binding.ArrayIndex,
					&gpuAddress, sizeof(gpuAddress));

				// B6: cache the resolved handle so the useResource: emission path does not re-scan the
				// per-type vectors to find this binding again. Index is the argIndex the layout handed
				// us above.
				if (resourceIndex < (u32)mResolvedResources.size())
					mResolvedResources[resourceIndex] = (__bridge void*)buffer;
			}

			for (const auto& binding : mStorageBuffers)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageBuffer, binding.Slot, binding.ArrayIndex);
				const u32 resourceIndex = mMetalLayout->GetResourceIndex(GpuParameterType::StorageBuffer, binding.Slot, binding.ArrayIndex);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlBuffer = std::static_pointer_cast<MetalGpuBuffer>(binding.Buffer);
				id<MTLBuffer> metalBuffer = mtlBuffer ? mtlBuffer->GetMetalBuffer() : nil;
				// Apply the view's byte offset so structured-buffer suballocations and dynamic offsets
				// (routed through SetDynamicOffset below) read from the correct slice. Vulkan encodes
				// the same offset into its VkDescriptorBufferInfo; encoding 0 here silently pointed
				// every storage binding at the start of the buffer.
				u64 gpuAddress = metalBuffer != nil ? (u64)metalBuffer.gpuAddress + binding.View.Offset : 0;
				fnWriteArgument(GpuParameterType::StorageBuffer, binding.Slot, binding.ArrayIndex,
					&gpuAddress, sizeof(gpuAddress));

				if (resourceIndex < (u32)mResolvedResources.size())
					mResolvedResources[resourceIndex] = (__bridge void*)metalBuffer;
			}

			for (const auto& binding : mSampledTextures)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::SampledTexture, binding.Slot, binding.ArrayIndex);
				const u32 resourceIndex = mMetalLayout->GetResourceIndex(GpuParameterType::SampledTexture, binding.Slot, binding.ArrayIndex);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlTexture = std::static_pointer_cast<MetalTexture>(binding.Texture);
				id<MTLTexture> tex = mtlTexture ? mtlTexture->GetMetalTexture() : nil;
				MTLResourceID resourceId = {};
				if(tex != nil)
					resourceId = tex.gpuResourceID;
				fnWriteArgument(GpuParameterType::SampledTexture, binding.Slot, binding.ArrayIndex,
					&resourceId, sizeof(resourceId));

				if (resourceIndex < (u32)mResolvedResources.size())
					mResolvedResources[resourceIndex] = (__bridge void*)tex;
			}

			for (const auto& binding : mStorageTextures)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::StorageTexture, binding.Slot, binding.ArrayIndex);
				const u32 resourceIndex = mMetalLayout->GetResourceIndex(GpuParameterType::StorageTexture, binding.Slot, binding.ArrayIndex);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlTexture = std::static_pointer_cast<MetalTexture>(binding.Texture);
				id<MTLTexture> tex = mtlTexture ? mtlTexture->GetMetalTexture() : nil;
				MTLResourceID resourceId = {};
				if(tex != nil)
					resourceId = tex.gpuResourceID;
				fnWriteArgument(GpuParameterType::StorageTexture, binding.Slot, binding.ArrayIndex,
					&resourceId, sizeof(resourceId));

				if (resourceIndex < (u32)mResolvedResources.size())
					mResolvedResources[resourceIndex] = (__bridge void*)tex;
			}

			for (const auto& binding : mSamplers)
			{
				const u32 argIndex = mMetalLayout->GetArgumentBufferIndex(GpuParameterType::Sampler, binding.Slot, binding.ArrayIndex);
				if (argIndex == (u32)~0u)
					continue;
				if (mDirtyArgumentSlots.find(argIndex) == mDirtyArgumentSlots.end())
					continue;

				auto mtlSampler = std::static_pointer_cast<MetalSamplerState>(binding.Sampler);
				id<MTLSamplerState> state = mtlSampler ? mtlSampler->GetMetalSampler() : nil;
				MTLResourceID resourceId = {};
				if(state != nil)
					resourceId = state.gpuResourceID;
				fnWriteArgument(GpuParameterType::Sampler, binding.Slot, binding.ArrayIndex,
					&resourceId, sizeof(resourceId));

				// Samplers never flow into useResource: (they are stage-inherent in Metal), so there is no
				// benefit to caching their handle — leave mResolvedResources[argIndex] as nullptr.
			}

			mDirtyArgumentSlots.clear();
			} // @autoreleasepool
			return mGeneration;
		}

		id<MTLResource> MetalGpuParameters::GetCachedResource(u32 resourceIndex) const
		{
			if (resourceIndex >= (u32)mResolvedResources.size())
				return nil;
			return (__bridge id<MTLResource>)mResolvedResources[resourceIndex];
		}
	} // namespace render
} // namespace b3d
