//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DIMetalRenderWindowSurface.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalTexture.h"
#include "B3DMetalUtility.h"
#include "Image/B3DPixelUtility.h"

namespace b3d::render
{
	MetalImage* IMetalRenderWindowSurface::CreateSurfaceImage(MetalGpuDevice& device, MTLPixelFormat format, PixelFormat engineFormat, TextureUsageFlags usage,
		u32 width, u32 height, const StringView& name)
	{
		MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:width height:height mipmapped:NO];
		descriptor.usage = usage.IsSet(TextureUsageFlag::DepthStencil) ? MTLTextureUsageRenderTarget : (MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead);
		descriptor.storageMode = MTLStorageModePrivate;
		descriptor.hazardTrackingMode = MTLHazardTrackingModeUntracked;

		MetalHeapAllocator& heapAllocator = device.GetHeapAllocator();
		const GpuAllocation requestedAllocation = GpuAllocation::CreatePending(heapAllocator.GetGpuMemoryAllocator(MetalMemoryType::Private));

		GpuAllocation allocation;
		id<MTLTexture> texture = heapAllocator.AllocateTexture(descriptor, requestedAllocation, allocation);
		if (texture == nil)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create the render window surface image '{0}' ({1}x{2}).", name, width, height);
			return nullptr;
		}

		MetalImageCreateInformation createInformation;
		createInformation.Format = engineFormat;
		createInformation.Usage = usage;
		createInformation.DebugName = name;

		MetalImage* image = device.GetResourceManager().Create<MetalImage>(createInformation, texture, allocation);
		image->SetName(name);

		return image;
	}

	TOptional<RenderWindowImageLayout> IMetalRenderWindowSurface::GetImageLayout() const
	{
		id<MTLTexture> colorTexture = GetCurrentColorTexture();
		if (colorTexture == nil)
			return {};

		// Only development builds configure the CAMetalLayer with framebufferOnly=NO so the drawable texture can be used as a blit
		// source. Blitting from a framebuffer-only texture silently produces undefined bytes on Apple Silicon and fails Metal API
		// validation.
		if (colorTexture.framebufferOnly == YES)
			return {};

		RenderWindowImageLayout layout;
		layout.Width = (u32)colorTexture.width;
		layout.Height = (u32)colorTexture.height;
		layout.Format = GetColorPixelFormat();

		const u32 rowPitchBytes = MetalUtility::GetTextureRowPitch(layout.Format, layout.Width);
		layout.Pitch = ImageSubresourcePitch(rowPitchBytes / PixelUtility::GetBlockSize(layout.Format), layout.Height);

		return layout;
	}
} // namespace b3d::render
