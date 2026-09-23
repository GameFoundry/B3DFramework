//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuParameterSet.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "B3DMetalGpuParameterSetPool.h"
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalResourceTracker.h"
#include "B3DMetalBarrierHelper.h"
#include "B3DMetalTexture.h"
#include "B3DMetalSamplerState.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		MetalArgumentBuffer::MetalArgumentBuffer(MetalResourceManager* owner, MetalBufferNativeHandle buffer, const GpuResourceLocation& allocation)
			: MetalResource(owner, StringView()), mBuffer(buffer), mAllocation(allocation)
		{ }

		MetalArgumentBuffer::~MetalArgumentBuffer()
		{
#if !__has_feature(objc_arc)
			[mBuffer release];
#endif
			mBuffer = nullptr;

			if (mAllocation.IsValid())
				mAllocation.Allocator->Free(mAllocation);
		}

		struct MetalGpuParameters::Impl
		{
			id<MTLBuffer> ArgumentBuffer = nil;

			/** Owner of ArgumentBuffer for persistent sets. Null when ArgumentBuffer is a transient pool slice. */
			MetalArgumentBuffer* ArgumentBufferResource = nullptr;

			/**
			 * Byte offset into @c ArgumentBuffer where this set's slice begins. Always zero for persistent sets.
			 * Direct writes and command-encoder binds add this to the reflected set-local byte offset.
			 */
			u64 ArgumentBufferOffset = 0;

			/**
			 * Sampler written into sampler entries that have no bound sampler. Plain BSL SamplerState declarations have
			 * no explicit engine-side binding; matching the Vulkan backend, they get the device's default sampler
			 * instead of a null resource ID (which samples as point filtering on affected hardware).
			 */
			TShared<MetalSamplerState> DefaultSampler;
		};

		namespace
		{
			/**
			 * Places a @p size-byte shared-storage argument buffer through the device's persistent heap allocator and
			 * wraps it in a tracked resource. Returns null on failure.
			 */
			MetalArgumentBuffer* CreateArgumentBuffer(MetalGpuDevice& device, u64 size)
			{
				GpuResourceLocation location;
				id<MTLBuffer> buffer = device.GetHeapAllocator().AllocateBuffer(size, MetalHeapAllocator::kMemoryTypeShared, location);
				if (buffer == nil)
					return nullptr;

				return device.GetResourceManager().Create<MetalArgumentBuffer>(buffer, location);
			}

			bool ValidateBufferRange(const TShared<GpuBuffer>& buffer, u32 offset, u32 range, const char* bindingType)
			{
				if (buffer == nullptr)
					return offset == 0 && range == 0;

				// Dynamic offsets address individual suballocations, so the valid range spans the whole buffer
				const u32 bufferSize = buffer->GetTotalSize();
				if ((offset & 15u) != 0 || offset >= bufferSize || (range != 0 && ((u64)offset + range > bufferSize)))
				{
					B3D_LOG(Error, LogRenderBackend,
						"Metal {0} binding has an invalid offset/range. Offset: {1}, range: {2}, buffer size: {3}. "
						"Offsets must be 16-byte aligned.", bindingType, offset, range, bufferSize);
					return false;
				}

				return true;
			}

			/** Converts a Tier-2 resource ID into the 64-bit value stored in the argument buffer. */
			u64 ToArgumentValue(MTLResourceID resourceId)
			{
				static_assert(sizeof(MTLResourceID) == sizeof(u64), "Tier-2 argument-buffer resource IDs must be 64-bit values.");

				u64 value;
				std::memcpy(&value, &resourceId, sizeof(value));
				return value;
			}

			/** Returns the stages that reference the binding of @p type at @p slot; no stages when the layout has no such binding. */
			GpuProgramStageBits GetBindingStages(const MetalGpuPipelineParameterSetLayout& layout, GpuParameterType type, u32 slot)
			{
				if (const MetalArgumentBufferBinding* binding = layout.FindBinding(type, slot))
					return binding->Stages;

				if (type == GpuParameterType::UniformBuffer)
				{
					if (const MetalDynamicUniformBufferBinding* binding = layout.FindDynamicUniformBufferBinding(slot))
						return binding->Stages;
				}

				return GpuProgramStageBit::None;
			}

			GpuResourceUseFlags GetBindingUseFlags(const MetalGpuPipelineParameterSetLayout& layout, GpuParameterType type, u32 slot, bool compute)
			{
				const GpuProgramStageBits stages = GetBindingStages(layout, type, slot);
				if (stages == GpuProgramStageBit::None)
					return GpuResourceUseFlag::Undefined;

				GpuResourceUseFlags useFlags = type == GpuParameterType::UniformBuffer
					? GpuResourceUseFlag::UniformBuffer
					: GpuResourceUseFlag::ShaderAccess;

				if (compute)
				{
					if (!stages.IsSet(GpuProgramStageBit::Compute))
						return GpuResourceUseFlag::Undefined;

					useFlags |= GpuResourceUseFlag::StageComputeShader;
				}
				else
				{
					if (stages.IsSetAny(GpuProgramStageBit::Vertex | GpuProgramStageBit::Hull | GpuProgramStageBit::Domain))
						useFlags |= GpuResourceUseFlag::StageVertexShader;

					if (stages.IsSet(GpuProgramStageBit::Fragment))
						useFlags |= GpuResourceUseFlag::StageFragmentShader;

					if (!useFlags.IsSetAny(GpuResourceUseFlag::StageVertexShader | GpuResourceUseFlag::StageFragmentShader))
						return GpuResourceUseFlag::Undefined;
				}

				return useFlags;
			}

			GpuAccessFlags GetStorageBufferAccessFlags(const MetalGpuPipelineParameterSetLayout& layout, u32 slot)
			{
				const MetalArgumentBufferBinding* binding = layout.FindBinding(GpuParameterType::StorageBuffer, slot);
				if (binding != nullptr && GpuObjectParameterTypeInformation::IsReadWriteBuffer(binding->ObjectType))
					return GpuAccessFlag::Read | GpuAccessFlag::Write;

				return GpuAccessFlag::Read;
			}
		} // namespace

		MetalGpuParameters::MetalGpuParameters(MetalGpuDevice& gpuDevice, const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex, MetalGpuParameterSetPool* pool)
			: GpuParameterSet(parameterSetLayout, setIndex), mGpuDevice(gpuDevice), mImpl(B3DMakeUnique<Impl>()), mPool(pool)
		{
			mMetalLayout = static_cast<const MetalGpuPipelineParameterSetLayout*>(parameterSetLayout.get());
		}

		MetalGpuParameters::~MetalGpuParameters()
		{
			if (mImpl && mImpl->ArgumentBufferResource != nullptr)
				mImpl->ArgumentBufferResource->Destroy();
		}

		void MetalGpuParameters::Initialize()
		{
			if (mMetalLayout != nullptr)
			{
				const u64 bufferSize = mMetalLayout->GetArgumentBufferSize();
				if (bufferSize != 0)
				{
					// Transient sets sub-allocate a slice from their pool, which recycles it wholesale on Reset().
					// Persistent sets place their own buffer through the heap allocator. Either way the storage
					// is shared, keeping the argument buffer CPU-writable without explicit synchronization.
					if (mPool != nullptr)
					{
						const u32 alignment = std::max<u32>(1u, mMetalLayout->GetArgumentBufferAlignment());
						u64 offset = 0;
						mImpl->ArgumentBuffer = mPool->AcquireArgumentBufferSlice(bufferSize, alignment, offset);
						mImpl->ArgumentBufferOffset = offset;
					}
					else
					{
						mImpl->ArgumentBufferResource = CreateArgumentBuffer(mGpuDevice, bufferSize);
						mImpl->ArgumentBuffer = mImpl->ArgumentBufferResource != nullptr ? mImpl->ArgumentBufferResource->GetMetalBuffer() : nil;
						mImpl->ArgumentBufferOffset = 0;
					}

					// Zeroed to match the default ArgumentElement::Value, so the first PrepareForBind writes every entry
					// whose resolved value is non-zero (dummy buffers and default samplers included).
					if (mImpl->ArgumentBuffer == nil)
						B3D_LOG(Error, LogRenderBackend, "Failed to allocate a {0}-byte Metal argument buffer for parameter set {1}.", bufferSize, GetSet());
					else
						std::memset((u8*)[mImpl->ArgumentBuffer contents] + mImpl->ArgumentBufferOffset, 0, (size_t)bufferSize);
				}

				mElements.assign((size_t)mMetalLayout->GetResourceCount(), ArgumentElement());
				mImpl->DefaultSampler = std::static_pointer_cast<MetalSamplerState>(mGpuDevice.FindOrCreateSamplerState(SamplerStateCreateInformation()));
			}

			GpuParameterSet::Initialize();
		}

		id<MTLBuffer> MetalGpuParameters::GetArgumentBuffer() const
		{
			return mImpl ? mImpl->ArgumentBuffer : nil;
		}

		MetalArgumentBuffer* MetalGpuParameters::GetArgumentBufferResource() const
		{
			return mImpl ? mImpl->ArgumentBufferResource : nullptr;
		}

		u64 MetalGpuParameters::GetArgumentBufferOffset() const
		{
			return mImpl ? mImpl->ArgumentBufferOffset : 0;
		}

		bool MetalGpuParameters::IsArgumentBufferAllocated() const
		{
			if (mMetalLayout == nullptr || mMetalLayout->GetArgumentBufferSize() == 0)
				return true;

			if (mImpl == nullptr || mImpl->ArgumentBuffer == nil)
				return false;

			return true;
		}

		bool MetalGpuParameters::SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex, u32 offset)
		{
			if (!ValidateBufferRange(uniformBuffer, offset, 0, "uniform-buffer"))
				return false;

			if (!GpuParameterSet::SetUniformBuffer(slot, uniformBuffer, arrayIndex, offset))
				return false;

			mBindingsDirty = true;
			return true;
		}

		bool MetalGpuParameters::SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
		{
			if (!GpuParameterSet::SetSampledTexture(slot, texture, surface, arrayIndex))
				return false;

			mBindingsDirty = true;
			return true;
		}

		bool MetalGpuParameters::SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
		{
			if (!GpuParameterSet::SetStorageTexture(slot, texture, surface, arrayIndex))
				return false;

			mBindingsDirty = true;
			return true;
		}

		bool MetalGpuParameters::SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex, GpuBufferViewInformation view)
		{
			const MetalArgumentBufferBinding* binding = mMetalLayout != nullptr ? mMetalLayout->FindBinding(GpuParameterType::StorageBuffer, slot) : nullptr;
			const GpuParameterObjectType objectType = binding != nullptr ? binding->ObjectType : GPOT_UNKNOWN;
			if (objectType != GPOT_STRUCTURED_BUFFER && objectType != GPOT_RWSTRUCTURED_BUFFER && objectType != GPOT_BYTE_BUFFER && objectType != GPOT_RWBYTE_BUFFER)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal storage buffers with counters are not supported. Slot: {0}.", slot);
				return false;
			}

			if (!ValidateBufferRange(buffer, view.Offset, view.Range, "storage-buffer"))
				return false;

			if (!GpuParameterSet::SetStorageBuffer(slot, buffer, arrayIndex, view))
				return false;

			mBindingsDirty = true;
			return true;
		}

		bool MetalGpuParameters::SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex)
		{
			if (!GpuParameterSet::SetSamplerState(slot, sampler, arrayIndex))
				return false;

			mBindingsDirty = true;
			return true;
		}

		GpuBuffer* MetalGpuParameters::GetBoundUniformBuffer(u32 slot, u32& outOffset) const
		{
			const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, 0);
			if (sequentialResourceIndex == ~0u)
			{
				outOffset = 0;
				return nullptr;
			}

			outOffset = mUniformBufferData[sequentialResourceIndex].Offset;
			return mUniformBufferData[sequentialResourceIndex].Buffer.get();
		}

		u64 MetalGpuParameters::PrepareForBind()
		{
			if (mMetalLayout == nullptr || !mImpl || mImpl->ArgumentBuffer == nil)
				return mGeneration;

			@autoreleasepool
			{
			const TArray<MetalArgumentBufferBinding>& bindings = mMetalLayout->GetBindings();

			// Buffers and textures can swap their backing Metal resource under a stable engine-side wrapper
			// (RecreateInternalBuffer / RecreateInternalTexture on discard writes) without any Set* call firing,
			// which would leave a stale handle in the argument buffer. Re-check the native handles at every bind.
			if (!mBindingsDirty)
			{
				for (const MetalArgumentBufferBinding& binding : bindings)
				{
					if (binding.Type == GpuParameterType::Sampler)
						continue;

					for (u32 arrayIndex = 0; arrayIndex < binding.ArraySize && !mBindingsDirty; arrayIndex++)
					{
						const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(binding.Slot, arrayIndex);

						void* nativeHandle = nullptr;
						switch (binding.Type)
						{
						case GpuParameterType::UniformBuffer:
							if (auto* buffer = static_cast<MetalGpuBuffer*>(mUniformBufferData[sequentialResourceIndex].Buffer.get()))
								nativeHandle = (__bridge void*)buffer->GetMetalBuffer();
							break;
						case GpuParameterType::StorageBuffer:
							if (auto* buffer = static_cast<MetalGpuBuffer*>(mStorageBufferData[sequentialResourceIndex].Buffer.get()))
								nativeHandle = (__bridge void*)buffer->GetMetalBuffer();
							break;
						case GpuParameterType::SampledTexture:
							if (auto* texture = static_cast<MetalTexture*>(mSampledTextureData[sequentialResourceIndex].Texture.get()))
								nativeHandle = (__bridge void*)texture->GetMetalTexture();
							break;
						case GpuParameterType::StorageTexture:
							if (auto* texture = static_cast<MetalTexture*>(mStorageTextureData[sequentialResourceIndex].Texture.get()))
								nativeHandle = (__bridge void*)texture->GetMetalTexture();
							break;
						default:
							break;
						}

						if (mElements[binding.FirstResourceIndex + arrayIndex].NativeHandle != nativeHandle)
							mBindingsDirty = true;
					}

					if (mBindingsDirty)
						break;
				}

				if (!mBindingsDirty)
					return mGeneration;
			}

			id<MTLBuffer> dummyBuffer = mGpuDevice.GetDummyArgumentBuffer();
			id<MTLSamplerState> defaultSampler = mImpl->DefaultSampler != nullptr ? mImpl->DefaultSampler->GetMetalSampler() : nil;

			// Gets writeable argument buffer memory. If the current argument buffer does not exist or is being used, a new buffer is allocated, and buffer generation incremented.
			const auto fnGetWritableArgumentBytes = [this]() -> u8*
			{
				const bool isArgumentBufferBound = mImpl->ArgumentBufferResource != nullptr ? mImpl->ArgumentBufferResource->IsBound() : mPool != nullptr;
				if (isArgumentBufferBound)
				{
					const u64 bufferSize = mMetalLayout->GetArgumentBufferSize();
					const u8* oldBytes = (const u8*)[mImpl->ArgumentBuffer contents] + mImpl->ArgumentBufferOffset;

					id<MTLBuffer> newBuffer = nil;
					MetalArgumentBuffer* newResource = nullptr;
					u64 newOffset = 0;
					if (mPool != nullptr)
					{
						const u32 alignment = std::max<u32>(1u, mMetalLayout->GetArgumentBufferAlignment());
						newBuffer = mPool->AcquireArgumentBufferSlice(bufferSize, alignment, newOffset);
					}
					else
					{
						newResource = CreateArgumentBuffer(mGpuDevice, bufferSize);
						newBuffer = newResource != nullptr ? newResource->GetMetalBuffer() : nil;
					}

					if (newBuffer != nil)
					{
						std::memcpy((u8*)[newBuffer contents] + newOffset, oldBytes, (size_t)bufferSize);

						if (mImpl->ArgumentBufferResource != nullptr)
							mImpl->ArgumentBufferResource->Destroy();

						mImpl->ArgumentBuffer = newBuffer;
						mImpl->ArgumentBufferResource = newResource;
						mImpl->ArgumentBufferOffset = newOffset;
						++mGeneration;
					}
					else
					{
						B3D_LOG(Error, LogRenderBackend,
							"Failed to copy-on-write a {0}-byte Metal argument buffer for parameter set {1}. "
							"Draws already recorded against it may read corrupted bindings.",
							mMetalLayout->GetArgumentBufferSize(), GetSet());
					}
				}

				return (u8*)[mImpl->ArgumentBuffer contents] + mImpl->ArgumentBufferOffset;
			};

			u8* argumentBytes = nullptr;
			bool resolvedResourcesChanged = false;
			for (const MetalArgumentBufferBinding& binding : bindings)
			{
				for (u32 arrayIndex = 0; arrayIndex < binding.ArraySize; arrayIndex++)
				{
					const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(binding.Slot, arrayIndex);

					u64 value = 0;
					id<MTLResource> resolvedResource = nil;
					void* nativeHandle = nullptr;
					switch (binding.Type)
					{
					case GpuParameterType::UniformBuffer:
					{
						const UniformBufferData& data = mUniformBufferData[sequentialResourceIndex];
						auto* buffer = static_cast<MetalGpuBuffer*>(data.Buffer.get());
						id<MTLBuffer> metalBuffer = buffer != nullptr ? buffer->GetMetalBuffer() : nil;
						nativeHandle = (__bridge void*)metalBuffer;

						if (metalBuffer == nil)
							metalBuffer = dummyBuffer;

						value = metalBuffer != nil ? (u64)metalBuffer.gpuAddress + data.Offset : 0;
						resolvedResource = metalBuffer;
						break;
					}
					case GpuParameterType::StorageBuffer:
					{
						const StorageBufferData& data = mStorageBufferData[sequentialResourceIndex];
						auto* buffer = static_cast<MetalGpuBuffer*>(data.Buffer.get());
						id<MTLBuffer> metalBuffer = buffer != nullptr ? buffer->GetMetalBuffer() : nil;
						nativeHandle = (__bridge void*)metalBuffer;

						if (binding.ObjectType == GPOT_BYTE_BUFFER || binding.ObjectType == GPOT_RWBYTE_BUFFER)
						{
							const bool writable = binding.ObjectType == GPOT_RWBYTE_BUFFER;
							id<MTLTexture> view = buffer != nullptr ? buffer->GetTextureBufferView(data.View.Format, data.View.Offset, data.View.Range, writable) : nil;

							value = view != nil ? ToArgumentValue(view.gpuResourceID) : 0;
							resolvedResource = view;
							break;
						}

						if (metalBuffer == nil)
							metalBuffer = dummyBuffer;

						value = metalBuffer != nil ? (u64)metalBuffer.gpuAddress + data.View.Offset : 0;
						resolvedResource = metalBuffer;
						break;
					}
					case GpuParameterType::SampledTexture:
					case GpuParameterType::StorageTexture:
					{
						const TextureData& data = binding.Type == GpuParameterType::SampledTexture ? mSampledTextureData[sequentialResourceIndex] : mStorageTextureData[sequentialResourceIndex];
						auto* texture = static_cast<MetalTexture*>(data.Texture.get());
						nativeHandle = texture != nullptr ? (__bridge void*)texture->GetMetalTexture() : nullptr;

						id<MTLTexture> view = texture != nullptr ? texture->GetSubresourceView(data.Surface) : nil;
						value = view != nil ? ToArgumentValue(view.gpuResourceID) : 0;
						resolvedResource = view;
						break;
					}
					case GpuParameterType::Sampler:
					{
						// Samplers are stage-inherent in Metal and never need residency, so they resolve to no resource
						auto* sampler = static_cast<MetalSamplerState*>(mSamplerStates[sequentialResourceIndex].get());
						id<MTLSamplerState> metalSampler = sampler != nullptr ? sampler->GetMetalSampler() : nil;
						if (metalSampler == nil)
							metalSampler = defaultSampler;

						value = metalSampler != nil ? ToArgumentValue(metalSampler.gpuResourceID) : 0;
						break;
					}
					default:
						break;
					}

					ArgumentElement& element = mElements[binding.FirstResourceIndex + arrayIndex];
					element.NativeHandle = nativeHandle;

					void* resolvedResourcePointer = (__bridge void*)resolvedResource;
					if (element.ResolvedResource != resolvedResourcePointer)
					{
						element.ResolvedResource = resolvedResourcePointer;
						resolvedResourcesChanged = true;
					}

					if (element.Value == value)
						continue;

					const u64 byteOffset = binding.ByteOffset + (u64)arrayIndex * binding.ByteStride;
					if (byteOffset + sizeof(value) > mMetalLayout->GetArgumentBufferSize())
					{
						B3D_LOG(Error, LogRenderBackend, "Metal argument-buffer write is outside the reflected layout. Set: {0}, slot: {1}, type: {2}.", GetSet(), binding.Slot, (u32)binding.Type);
						continue;
					}

					if (argumentBytes == nullptr)
						argumentBytes = fnGetWritableArgumentBytes();

					std::memcpy(argumentBytes + byteOffset, &value, sizeof(value));
					element.Value = value;
				}
			}

			// The resident resource set changed — force the command buffer to re-emit useResources:
			if (resolvedResourcesChanged)
				++mGeneration;

			mBindingsDirty = false;
			} // @autoreleasepool

			return mGeneration;
		}

		bool MetalGpuParameters::TrackResources(MetalResourceTracker& resourceTracker, MetalBarrierHelper& barrierHelper, const TInlineArray<u32, 4>& dynamicOffsetOverrides, bool compute) const
		{
			if (mMetalLayout == nullptr)
				return true;

			const MetalGpuPipelineParameterSetLayout& layout = *mMetalLayout;

			// Walks every binding of @p type in the engine layout (including dynamic-offset uniform buffers, which have
			// no argument-buffer entry), calling @p fnVisit with the binding's slot, array size and use flags. Bindings
			// the tracked stages never read are skipped.
			const auto fnForEachBinding = [this, &layout, compute](GpuParameterType type, const auto& fnVisit)
			{
				const u32 bindingCount = mParameterSetLayout->GetBindingCount(type);
				for (u32 bindingIndex = 0; bindingIndex < bindingCount; bindingIndex++)
				{
					const u32 slot = mParameterSetLayout->GetSlot(type, bindingIndex);
					const GpuResourceUseFlags useFlags = GetBindingUseFlags(layout, type, slot, compute);
					if (useFlags == GpuResourceUseFlag::Undefined)
						continue;

					if (!fnVisit(slot, mParameterSetLayout->GetArraySize(type, bindingIndex), useFlags))
						return false;
				}

				return true;
			};

			const bool trackedUniformBuffers = fnForEachBinding(GpuParameterType::UniformBuffer,
				[&](u32 slot, u32 arraySize, GpuResourceUseFlags useFlags)
				{
					for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
					{
						const UniformBufferData& data = mUniformBufferData[mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex)];
						auto* buffer = static_cast<MetalGpuBuffer*>(data.Buffer.get());
						MetalBuffer* resource = buffer != nullptr ? buffer->GetMetalResource() : nullptr;
						if (resource == nullptr)
							continue;

						u32 offset = data.Offset;
						const u32 dynamicOffsetIndex = mParameterSetLayout->GetDynamicOffsetIndex(slot, arrayIndex);
						if (dynamicOffsetIndex < (u32)dynamicOffsetOverrides.Size() && dynamicOffsetOverrides[dynamicOffsetIndex] != ~0u)
							offset = dynamicOffsetOverrides[dynamicOffsetIndex];

						resourceTracker.TrackBufferAccess(resource, GpuBackendUtility::GetStageFlags(useFlags), GpuAccessFlag::Read, barrierHelper, offset);
					}

					return true;
				});

			const bool trackedStorageBuffers = fnForEachBinding(GpuParameterType::StorageBuffer,
				[&](u32 slot, u32 arraySize, GpuResourceUseFlags useFlags)
				{
					const GpuAccessFlags accessFlags = GetStorageBufferAccessFlags(layout, slot);
					for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
					{
						const StorageBufferData& data = mStorageBufferData[mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex)];
						auto* buffer = static_cast<MetalGpuBuffer*>(data.Buffer.get());
						MetalBuffer* resource = buffer != nullptr ? buffer->GetMetalResource() : nullptr;
						if (resource != nullptr)
							resourceTracker.TrackBufferAccess(resource, GpuBackendUtility::GetStageFlags(useFlags), accessFlags, barrierHelper, data.View.Offset);
					}

					return true;
				});

			const auto fnTrackTextures = [&](GpuParameterType type, const TextureData* textureData, GpuAccessFlags access)
			{
				return fnForEachBinding(type,
					[&](u32 slot, u32 arraySize, GpuResourceUseFlags useFlags)
					{
						for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
						{
							const TextureData& data = textureData[mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex)];
							auto* texture = static_cast<MetalTexture*>(data.Texture.get());
							MetalImage* resource = texture != nullptr ? texture->GetMetalResource() : nullptr;
							if (resource == nullptr)
								continue;

							GpuTextureSubresourceRange range = resource->GetRange(data.Surface);
							// GetRange() reports resource aspects, while sampling a depth-stencil texture only reads the
							// depth plane. Keep the tracked range to that plane so a read-only depth attachment can be sampled
							// while its stencil aspect is still being written.
							if (type == GpuParameterType::SampledTexture && range.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
								range.AspectMask = GpuTextureAspectFlag::Depth;

							const GpuImageLayout imageLayout = type == GpuParameterType::StorageTexture ? GpuImageLayout::General : GpuImageLayout::ShaderReadOnly;
							if (!resourceTracker.TrackImageUsage(resource, range, imageLayout, useFlags, access, barrierHelper))
								return false;
						}

						return true;
					});
			};

			return trackedUniformBuffers && trackedStorageBuffers
				&& fnTrackTextures(GpuParameterType::SampledTexture, mSampledTextureData, GpuAccessFlag::Read)
				&& fnTrackTextures(GpuParameterType::StorageTexture, mStorageTextureData, GpuAccessFlag::Read | GpuAccessFlag::Write);
		}

		id<MTLResource> MetalGpuParameters::GetCachedResource(u32 resourceIndex) const
		{
			if (resourceIndex >= (u32)mElements.size())
				return nil;

			return (__bridge id<MTLResource>)mElements[resourceIndex].ResolvedResource;
		}
	} // namespace render
} // namespace b3d
