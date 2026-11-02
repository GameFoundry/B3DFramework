---
title: Bindless resources
---

Bindless resources let GPU programs read textures, buffers and samplers by index, without binding them through a parameter set. Each GPU device that supports them has two global heaps: a resource heap and a sampler heap. A resource or sampler is written into a heap slot once, and programs read it through the slot's index for as long as it lives.

Check for support with @b3d::GpuDeviceCapabilities::HasCapability and @b3d::RSC_BINDLESS_RESOURCES.

# Accessing the heaps in a program

Programs index the heaps with the Shader Model 6.6 `ResourceDescriptorHeap` and `SamplerDescriptorHeap` objects. The index usually comes from push constants, or from a buffer.

~~~~~~~~~~~~~{.hlsl}
[pushConstant] cbuffer Indices
{
	uint MaterialTextureIndex;
	uint MaterialSamplerIndex;
};

float4 fsmain(VStoFS input) : SV_Target0
{
	Texture2D<float4> materialTexture = ResourceDescriptorHeap[MaterialTextureIndex];
	SamplerState materialSampler = SamplerDescriptorHeap[MaterialSamplerIndex];

	return materialTexture.Sample(materialSampler, input.uv);
}
~~~~~~~~~~~~~

Functions can return resources and samplers read from the heaps, on their own or as members of a struct.

~~~~~~~~~~~~~{.hlsl}
Texture2D<float4> GetMaterialTexture(uint materialIndex)
{
	return ResourceDescriptorHeap[MaterialTextureIndex + materialIndex];
}
~~~~~~~~~~~~~

The resource heap holds read-only views only:
 - textures, as any `Texture*` type,
 - simple storage buffers, as `Buffer<T>`,
 - structured storage buffers, as `StructuredBuffer<T>`, or as raw 32-bit words through `ByteAddressBuffer`.

`RW*` types, append and consume buffers and constant buffers can't be read through the heap. Bind them through a parameter set as usual. The heaps are bound at parameter set 7 (register space 7 in HLSL), so programs that access them can't declare their own parameters there.

The pipeline binds the heaps automatically when one of its programs accesses them. Nothing needs to be set on the command buffer.

On Metal, two more restrictions apply:
 - `GetDimensions()` can't be called on a `StructuredBuffer<T>` or `ByteAddressBuffer` read through the heap.
 - A program that accesses `SamplerDescriptorHeap` must leave at least one of parameter sets 0 to 6 unused, because the sampler heap takes that set's place in the generated Metal shader.

# Bindless textures and buffers

Create a texture with the @b3d::TextureUsageFlag::Bindless usage flag, or a buffer with the @b3d::GpuBufferFlag::Bindless flag, to give it a heap slot. The slot of its default view is allocated on creation, and @b3d::render::Texture::GetBindlessIndex or @b3d::render::GpuBuffer::GetBindlessIndex return its index.

~~~~~~~~~~~~~{.cpp}
TextureCreateInformation textureInformation;
textureInformation.Width = 256;
textureInformation.Height = 256;
textureInformation.Format = PF_RGBA8;
textureInformation.Usage = TextureUsageFlag::Default | TextureUsageFlag::Bindless;

TShared<Texture> texture = gpuDevice->CreateTexture(textureInformation);
u32 textureIndex = texture->GetBindlessIndex();

TShared<GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(Vector4), 1024,
	GpuBufferFlag::StoreOnGPU | GpuBufferFlag::Bindless));
u32 bufferIndex = buffer->GetBindlessIndex();
~~~~~~~~~~~~~

Other views get their own slots on first request:
 - @b3d::render::Texture::GetOrCreateBindlessIndex views a @b3d::TextureSurface of the texture, such as a single mip level.
 - @b3d::render::GpuBuffer::GetOrCreateBindlessIndex views a simple storage buffer with another element format (@b3d::GpuBufferViewType::Typed), or a structured storage buffer as raw 32-bit words (@b3d::GpuBufferViewType::Raw), which programs read as a `ByteAddressBuffer`.

~~~~~~~~~~~~~{.cpp}
// View of the first mip level only
u32 mipIndex = texture->GetOrCreateBindlessIndex(TextureSurface(0, 1, 0, 1));

// Raw view of the structured buffer
u32 rawBufferIndex = buffer->GetOrCreateBindlessIndex(GpuBufferViewType::Raw);
~~~~~~~~~~~~~

~~~~~~~~~~~~~{.hlsl}
ByteAddressBuffer rawBuffer = ResourceDescriptorHeap[RawBufferIndex];
uint4 words = rawBuffer.Load4(byteOffset);
~~~~~~~~~~~~~

Indices are valid until the resource is destroyed. The slots are reused once the GPU finishes the frame in which the resource was destroyed, so work recorded before the destruction can still read them. Index 0 is never allocated in either heap. It is @b3d::kGpuBindlessInvalidIndex, which the getters return when the resource has no slot.

A command buffer that accesses the heaps must finish recording in the frame it started recording in. At the end of a frame the device may switch to a new native heap, and indices allocated afterwards are only valid in work recorded afterwards. @b3d::render::GpuCommandBuffer::End logs an error for a command buffer recorded across the end of a frame in which the heap switched.

A bindless resource must:
 - be a texture, or a simple or structured storage buffer, with no suballocations,
 - not be aliased or transient, so it can't come from the transient resource allocator or a frame graph,
 - stay at rest. Programs read it through the heap without barriers, so any pass that writes it must return it to its resting state, as frame graph passes do for imported resources.

Writes to bindless resources never discard their memory. Defragmentation may move them (see @b3d::GpuMemoryDefragmenter), and updates their slots to the moved memory at the end of the frame, so their indices stay valid.

# Samplers

Every sampler state has a slot in the sampler heap, allocated the first time @b3d::SamplerState::GetBindlessIndex is called. Sampler states are cached by the device, so samplers created with the same @b3d::SamplerStateCreateInformation share one slot.

~~~~~~~~~~~~~{.cpp}
SamplerStateCreateInformation samplerInformation;
samplerInformation.MaxAniso = 8;

TShared<SamplerState> sampler = gpuDevice->FindOrCreateSamplerState(samplerInformation);
u32 samplerIndex = sampler->GetBindlessIndex();
~~~~~~~~~~~~~

When the sampler heap fills past `gpu.BindlessSamplerEvictionThreshold`, the device evicts cached samplers that nothing else references, which frees their slots.

## Global samplers

Six common samplers have permanent slots, at the indices returned by @b3d::GetGlobalSamplerIndex. They combine point, bilinear or trilinear filtering (@b3d::GpuGlobalSamplerFilter) with wrap or clamp addressing (@b3d::GpuGlobalSamplerAddress). Programs can use them without being passed an index: the `BindlessSamplers` mixin provides the samplers through `GetGlobalSampler()`, their indices through `GetGlobalSamplerIndex()`, and the `GLOBAL_SAMPLER_*` constants.

~~~~~~~~~~~~~{.hlsl}
#include "$ENGINE$\BindlessSamplers.bslinc"

shader MyShader
{
	mixin BindlessSamplers;

	code
	{
		float4 fsmain(VStoFS input) : SV_Target0
		{
			Texture2D<float4> materialTexture = ResourceDescriptorHeap[MaterialTextureIndex];
			SamplerState linearClamp = GetGlobalSampler(GLOBAL_SAMPLER_FILTER_BILINEAR, GLOBAL_SAMPLER_ADDRESS_CLAMP);

			return materialTexture.Sample(linearClamp, input.uv);
		}
	};
};
~~~~~~~~~~~~~

# Configuration

The resource heap grows as it fills, and the sampler heap has a fixed capacity:
 - `gpu.BindlessResourceCapacity` (default 65536) is the initial number of resource heap slots.
 - `gpu.BindlessResourceMaximumCapacity` (default 1048576) is the number of slots the resource heap may grow to.
 - `gpu.BindlessResourceGrowthHeadroom` (default 0.25) is the fraction of the resource heap that must stay free. When fewer slots are free at the end of a frame, the heap doubles, up to its maximum capacity.
 - `gpu.BindlessSamplerCapacity` (default 1024) is the number of sampler heap slots, including the global samplers.

The capacities are limited by the device: see @b3d::GpuDeviceCapabilities::MaximumBindlessResourceCount and @b3d::GpuDeviceCapabilities::MaximumBindlessSamplerCount. Indices stay valid when the heap grows, and growth needs nothing from the caller.

Slots added by a growth can be allocated right away. Allocation fails with an error when the resource heap is full, either because it is at its maximum capacity (raise `gpu.BindlessResourceMaximumCapacity`), or because more slots were allocated in one frame than the headroom (raise `gpu.BindlessResourceCapacity` or `gpu.BindlessResourceGrowthHeadroom`). On Metal, the resource heap is created at its maximum capacity and never grows.

@b3d::GpuBindlessHeaps::GetStatistics, available through @b3d::GpuDevice::GetBindlessHeaps, reports how many slots are in use, the current and maximum capacities, and how many times the heap grew.

In development builds, slots still allocated when the device shuts down are reported as leaks, with the name of the resource or sampler that owns them. Retired slots are also cleared, so stale indices read zeros:
 - A retired resource slot gets a null descriptor of the same type as the view it held (texture dimension, typed, structured or raw buffer), as reads through a null descriptor of a different type are undefined. On Vulkan this needs `VK_EXT_robustness2` with the `nullDescriptor` feature. Without it, retired slots hold an empty storage buffer, so only stale structured and raw buffer indices read zeros.
 - A retired sampler slot gets the point filtered, wrapping global sampler, as samplers have no null descriptor. This happens in all builds.
