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

The resource heap holds read-only views only:
 - textures, as any `Texture*` type,
 - simple storage buffers, as `Buffer<T>`,
 - structured storage buffers, as `StructuredBuffer<T>`.

`RW*` types, append and consume buffers, `ByteAddressBuffer` and constant buffers can't be read through the heap. Bind them through a parameter set as usual. The heaps are bound at parameter set 7 (register space 7 in HLSL), so programs that access them can't declare their own parameters there.

The pipeline binds the heaps automatically when one of its programs accesses them. Nothing needs to be set on the command buffer.

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
 - @b3d::render::GpuBuffer::GetOrCreateBindlessIndex views a simple storage buffer with another element format.

~~~~~~~~~~~~~{.cpp}
// View of the first mip level only
u32 mipIndex = texture->GetOrCreateBindlessIndex(TextureSurface(0, 1, 0, 1));
~~~~~~~~~~~~~

Indices are valid until the resource is destroyed. The slots are reused once the GPU finishes the frame in which the resource was destroyed, so work recorded before the destruction can still read them. Index 0 is never allocated in either heap. It is @b3d::kGpuBindlessInvalidIndex, which the getters return when the resource has no slot.

A bindless resource must:
 - be a texture, or a simple or structured storage buffer, with no suballocations,
 - not be aliased or transient, so it can't come from the transient resource allocator or a frame graph,
 - stay at rest. Programs read it through the heap without barriers, so any pass that writes it must return it to its resting state, as frame graph passes do for imported resources.

Bindless resources are never moved by defragmentation, and writes to them never discard their memory, so their indices stay valid.

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

Six common samplers have permanent slots, at the indices returned by @b3d::GetGlobalSamplerIndex. They combine point, bilinear or trilinear filtering (@b3d::GpuGlobalSamplerFilter) with wrap or clamp addressing (@b3d::GpuGlobalSamplerAddress). Programs can use them without being passed an index: the `BindlessSamplers` mixin provides the same indices through `GetGlobalSamplerIndex()` and the `GLOBAL_SAMPLER_*` constants.

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
			SamplerState linearClamp = SamplerDescriptorHeap[GetGlobalSamplerIndex(GLOBAL_SAMPLER_FILTER_BILINEAR, GLOBAL_SAMPLER_ADDRESS_CLAMP)];

			return materialTexture.Sample(linearClamp, input.uv);
		}
	};
};
~~~~~~~~~~~~~

# Configuration

The heaps have a fixed capacity, set when the device is created:
 - `gpu.BindlessResourceCapacity` (default 262144) is the number of resource heap slots.
 - `gpu.BindlessSamplerCapacity` (default 1024) is the number of sampler heap slots, including the global samplers.

Both are limited by the device: see @b3d::GpuDeviceCapabilities::MaximumBindlessResourceCount and @b3d::GpuDeviceCapabilities::MaximumBindlessSamplerCount. Allocation fails with an error once a heap is full. @b3d::GpuBindlessHeaps::GetStatistics, available through @b3d::GpuDevice::GetBindlessHeaps, reports how many slots are in use.

In development builds, retired slots are cleared so stale indices read null data, and slots still allocated when the device shuts down are reported as leaks, with the name of the resource or sampler that owns them.
