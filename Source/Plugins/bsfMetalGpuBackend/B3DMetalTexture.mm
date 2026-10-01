//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalTexture.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalUtility.h"
#include "Image/B3DPixelUtility.h"
#include "Debug/B3DLog.h"
#include "Math/B3DMath.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Determines the full aspect flags for a texture with the provided @p usage and @p format. */
			GpuTextureAspectFlags GetFullAspectFlags(TextureUsageFlags usage, PixelFormat format)
			{
				if (usage.IsSet(TextureUsageFlag::DepthStencil))
				{
					// PF_D32_S8X24 is the only combined depth-stencil format the Metal pixel-format
					// table maps (MTLPixelFormatDepth32Float_Stencil8); PF_D16 / PF_D32 are depth-only.
					const bool hasStencil = format == PF_D32_S8X24;

					return hasStencil ? (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil) : GpuTextureAspectFlags(GpuTextureAspectFlag::Depth);
				}

				return GpuTextureAspectFlag::Color;
			}
		}

		MetalImage::MetalImage(MetalResourceManager* owner, const MetalImageCreateInformation& createInformation, MetalTextureNativeHandle texture, const GpuAllocation& allocation)
			: TMetalResource<IGpuImageResource>(owner, createInformation.DebugName, createInformation.FaceCount, createInformation.MipLevelCount, GetFullAspectFlags(createInformation.Usage, createInformation.Format)), mTexture(texture), mAllocation(allocation)
		{
			// Metal has no image layouts, so the native state only holds the tracked layout. New image contents are undefined.
			GpuImageNativeState initialNativeState;
			initialNativeState.Layout = (u32)GpuImageLayout::Undefined;
			InitializeNativeState(mFullRange.AspectMask, initialNativeState);
		}

		MetalImage::~MetalImage()
		{
			{
				Lock lock(mViewCacheMutex);
#if !__has_feature(objc_arc)
				for (auto& viewEntry : mShaderReadViews)
					[viewEntry.second release];
				for (auto& viewEntry : mSubresourceViews)
					[viewEntry.second release];
#endif
				mShaderReadViews.clear();
				mSubresourceViews.clear();
			}

#if !__has_feature(objc_arc)
			[mTexture release];
#endif
			mTexture = nullptr;

			if (mAllocation.IsOwned())
				mAllocation.Allocator->Free(mAllocation);
		}

		void MetalImage::SetName(const StringView& name)
		{
			if (mTexture == nullptr)
				return;

			@autoreleasepool
			{
				const String nameCopy(name.data(), name.size());
				[mTexture setLabel:[NSString stringWithUTF8String:nameCopy.c_str()]];
			}
		}

		id<MTLTexture> MetalImage::GetShaderReadView(MTLPixelFormat viewFormat)
		{
			if (mTexture == nil)
				return nil;

			if (viewFormat == [mTexture pixelFormat])
				return mTexture;

			Lock lock(mViewCacheMutex);

			const u32 key = (u32)viewFormat;
			auto existing = mShaderReadViews.find(key);
			if (existing != mShaderReadViews.end())
				return existing->second;

			const MTLPixelFormat parentFormat = [mTexture pixelFormat];

			const bool parentIsCombinedDS = (parentFormat == MTLPixelFormatDepth32Float_Stencil8) || (parentFormat == MTLPixelFormatDepth24Unorm_Stencil8);
			const bool viewIsSingleAspect = (viewFormat == MTLPixelFormatDepth32Float) || (viewFormat == MTLPixelFormatDepth16Unorm)
				|| (viewFormat == MTLPixelFormatStencil8) || (viewFormat == MTLPixelFormatX24_Stencil8) || (viewFormat == MTLPixelFormatX32_Stencil8);

			id<MTLTexture> view = nil;
			if (parentIsCombinedDS && viewIsSingleAspect)
			{
				const MTLTextureType textureType = [mTexture textureType];
				const bool isCube = textureType == MTLTextureTypeCube || textureType == MTLTextureTypeCubeArray;
				const NSUInteger sliceCount = [mTexture arrayLength] * (isCube ? 6 : 1);

				view = [mTexture newTextureViewWithPixelFormat:viewFormat textureType:textureType levels:NSMakeRange(0, [mTexture mipmapLevelCount]) slices:NSMakeRange(0, sliceCount)];
			}
			else
				view = [mTexture newTextureViewWithPixelFormat:viewFormat];

			if (view == nil)
			{
				B3D_LOG(Warning, LogRenderBackend, "Failed to create MTLTexture view with format {0}.", (u32)viewFormat);
				return nil;
			}

			mShaderReadViews[key] = view;
			return view;
		}

		id<MTLTexture> MetalImage::GetSubresourceView(const TextureSurface& surface)
		{
			if (mTexture == nil)
				return nil;

			const u32 mipLevelCount = (u32)[mTexture mipmapLevelCount];
			const MTLTextureType parentType = [mTexture textureType];
			const bool isCube = parentType == MTLTextureTypeCube || parentType == MTLTextureTypeCubeArray;
			const u32 sliceCount = (u32)[mTexture arrayLength] * (isCube ? 6 : 1);

			B3D_ASSERT(surface.MipLevel < mipLevelCount && surface.Face < sliceCount);
			const u32 firstMip = Math::Min(surface.MipLevel, mipLevelCount - 1);
			const u32 mipCount = surface.MipLevelCount == 0 ? mipLevelCount - firstMip : Math::Min(surface.MipLevelCount, mipLevelCount - firstMip);
			const u32 firstSlice = Math::Min(surface.Face, sliceCount - 1);
			const u32 viewSliceCount = surface.FaceCount == 0 ? sliceCount - firstSlice : Math::Min(surface.FaceCount, sliceCount - firstSlice);

			const bool coversWholeResource = firstMip == 0 && mipCount == mipLevelCount && firstSlice == 0 && viewSliceCount == sliceCount;
			if (coversWholeResource && !(isCube && surface.IsBoundAs2DArray))
				return mTexture;

			Lock lock(mViewCacheMutex);

			const u64 key = ((u64)(surface.IsBoundAs2DArray ? 1 : 0) << 60) | ((u64)firstMip << 52) | ((u64)mipCount << 44) | ((u64)firstSlice << 22) | (u64)viewSliceCount;
			auto existing = mSubresourceViews.find(key);
			if (existing != mSubresourceViews.end())
				return existing->second;

			// Re-type the view to match how the sliced range is addressed from a shader
			MTLTextureType viewType = parentType;
			switch (parentType)
			{
			case MTLTextureTypeCube:
			case MTLTextureTypeCubeArray:
				if (viewSliceCount == 1)
					viewType = MTLTextureType2D;
				else if ((viewSliceCount % 6) == 0)
				{
					if (surface.IsBoundAs2DArray)
						viewType = MTLTextureType2DArray;
					else
						viewType = viewSliceCount > 6 ? MTLTextureTypeCubeArray : MTLTextureTypeCube;
				}
				else
					viewType = MTLTextureType2DArray;
				break;
			case MTLTextureType1D:
				if (viewSliceCount > 1)
					viewType = MTLTextureType1DArray;
				break;
			case MTLTextureType2D:
				if (viewSliceCount > 1)
					viewType = MTLTextureType2DArray;
				break;
			default:
				break;
			}

			id<MTLTexture> view = [mTexture newTextureViewWithPixelFormat:[mTexture pixelFormat] textureType:viewType levels:NSMakeRange(firstMip, mipCount) slices:NSMakeRange(firstSlice, viewSliceCount)];
			if (view == nil)
			{
				B3D_LOG(Warning, LogRenderBackend, "Failed to create MTLTexture subresource view. Mips: [{0}, {1}), slices: [{2}, {3}).", firstMip, firstMip + mipCount, firstSlice, firstSlice + viewSliceCount);
				return nil;
			}

			mSubresourceViews[key] = view;
			return view;
		}

		MetalTexture::MetalTexture(MetalGpuDevice& gpuDevice, const TextureCreateInformation& createInformation, const GpuAllocation& allocation)
			: Texture(createInformation, allocation), mGpuDevice(gpuDevice)
		{ }

		MetalTexture::~MetalTexture()
		{
			if (mImage != nullptr)
				mImage->Destroy();
		}

		id<MTLTexture> MetalTexture::GetMetalTexture() const
		{
			return mImage != nullptr ? mImage->GetMetalHandle() : nil;
		}

		id<MTLTexture> MetalTexture::GetShaderReadView(MTLPixelFormat viewFormat)
		{
			return mImage != nullptr ? mImage->GetShaderReadView(viewFormat) : nil;
		}

		id<MTLTexture> MetalTexture::GetSubresourceView(const TextureSurface& surface)
		{
			return mImage != nullptr ? mImage->GetSubresourceView(surface) : nil;
		}

		void MetalTexture::SetName(const StringView& name)
		{
			Texture::SetName(name);

			if (mImage != nullptr)
				mImage->SetName(name);
		}

		void MetalTexture::Initialize()
		{
			mImage = CreateImage();

			if (mImage == nullptr)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalTexture allocation failed; skipping pixel upload for texture '{0}'. Texture is unusable.", GetName());
				return;
			}

			Texture::Initialize();
		}

		void MetalTexture::RecreateInternalTexture()
		{
			MetalImage* newImage = CreateImage();

			if (mImage != nullptr)
				mImage->Destroy();

			mImage = newImage;
		}

		MTLTextureDescriptor* MetalTexture::CreateDescriptor(id<MTLDevice> device, const TextureProperties& properties)
		{
			bool useSRGB = properties.UseHardwareSRGB;
			MTLPixelFormat mtlFormat = MetalUtility::GetPixelFormat(properties.Format, useSRGB);
			if (mtlFormat == MTLPixelFormatInvalid && useSRGB)
			{
				// Retry without sRGB; match the Vulkan backend's behavior where the linear variant
				// is used if the hardware cannot honor the sRGB request.
				B3D_LOG(Warning, LogRenderBackend, "MTLPixelFormat for format {0} unavailable in sRGB variant; falling back to linear.", (u32)properties.Format);
				useSRGB = false;
				mtlFormat = MetalUtility::GetPixelFormat(properties.Format, false);
			}

			if (mtlFormat == MTLPixelFormatInvalid)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create MTLTexture: unsupported pixel format {0}.", (u32)properties.Format);
				return nil;
			}

			if (PixelUtility::IsCompressed(properties.Format) && ![device supportsBCTextureCompression])
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create compressed MTLTexture: this Apple GPU does not support BC texture compression.");
				return nil;
			}

			if (properties.Width == 0 || (properties.Type != TEX_TYPE_1D && properties.Height == 0) || (properties.Type == TEX_TYPE_3D && properties.Depth == 0))
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create an MTLTexture with a zero relevant dimension.");
				return nil;
			}

			const u32 width = properties.Width;
			const u32 height = properties.Type == TEX_TYPE_1D ? 1u : properties.Height;
			const u32 depth = properties.Type == TEX_TYPE_3D ? properties.Depth : 1u;
			const u32 mipCount = properties.MipMapCount + 1;
			const u32 sampleCount = std::max(1u, properties.SampleCount);

			if (properties.ArraySliceCount == 0)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create MTLTexture with zero array slices.");
				return nil;
			}

			if ((properties.Type == TEX_TYPE_1D && (properties.Height > 1 || properties.Depth > 1)) ||
				((properties.Type == TEX_TYPE_2D || properties.Type == TEX_TYPE_CUBE_MAP) && properties.Depth > 1))
			{
				B3D_LOG(Error, LogRenderBackend, "MTLTexture dimensions do not match the requested texture type.");
				return nil;
			}

			if (properties.Type == TEX_TYPE_CUBE_MAP && width != height)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create a non-square Metal cube texture ({0}x{1}).", width, height);
				return nil;
			}

			u32 maximumMipCount = 1;
			for (u32 maximumDimension = std::max(width, std::max(height, depth)); maximumDimension > 1; maximumDimension >>= 1)
				maximumMipCount++;

			if (mipCount > maximumMipCount)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create MTLTexture with {0} mip levels; dimensions allow at most {1}.", mipCount, maximumMipCount);
				return nil;
			}

			if (sampleCount > 1 && ![device supportsTextureSampleCount:sampleCount])
			{
				B3D_LOG(Error, LogRenderBackend, "MTLDevice does not support texture sample count {0}.", sampleCount);
				return nil;
			}

			if (sampleCount > 1 && mipCount > 1)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal does not support multisampled textures with mipmaps.");
				return nil;
			}

			if (sampleCount > 1 && properties.Type != TEX_TYPE_2D)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal multisampling is supported only for 2D textures.");
				return nil;
			}

			MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
			descriptor.textureType = MetalUtility::GetTextureType(properties.Type, sampleCount, properties.ArraySliceCount);
			descriptor.pixelFormat = mtlFormat;
			descriptor.width = width;
			descriptor.height = height;
			descriptor.depth = depth;
			descriptor.mipmapLevelCount = mipCount;
			descriptor.sampleCount = sampleCount;
			descriptor.arrayLength = (properties.Type == TEX_TYPE_3D) ? 1 : properties.ArraySliceCount;

			MTLTextureUsage usage = MTLTextureUsageShaderRead;
			if (properties.Usage.IsSet(TextureUsageFlag::MutableFormat) || properties.Format == PF_D32_S8X24)
				usage |= MTLTextureUsagePixelFormatView;

			if (properties.Usage & TextureUsageFlag::RenderTarget)
				usage |= MTLTextureUsageRenderTarget;

			if (properties.Usage & TextureUsageFlag::DepthStencil)
				usage |= MTLTextureUsageRenderTarget;

			if (properties.Usage & TextureUsageFlag::AllowUnorderedAccessOnTheGPU)
				usage |= MTLTextureUsageShaderWrite;

			descriptor.usage = usage;
			descriptor.storageMode = MTLStorageModePrivate;

			// Direct textures and placement-heap textures use the same configured hazard policy.
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			descriptor.hazardTrackingMode = MTLHazardTrackingModeUntracked;
#else
			descriptor.hazardTrackingMode = MTLHazardTrackingModeTracked;
#endif

			return descriptor;
		}

		MetalImage* MetalTexture::CreateImage()
		{
			@autoreleasepool
			{
			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
				return nullptr;

			MTLTextureDescriptor* descriptor = CreateDescriptor(device, mProperties);
			if (descriptor == nil)
				return nullptr;

			GpuAllocation allocation;
			MetalTextureNativeHandle handle = mGpuDevice.GetHeapAllocator().AllocateTexture(descriptor, mRequestedAllocation, allocation);
#if !__has_feature(objc_arc)
			[descriptor release];
#endif

			if (handle == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create MTLTexture (format {0}, {1}x{2}).", (u32)mProperties.Format, mProperties.Width, mProperties.Height);
				return nullptr;
			}

			mInternalFormat = mProperties.Format;

			const u32 mipCount = mProperties.MipMapCount + 1;

			MetalImageCreateInformation imageCreateInformation;
			imageCreateInformation.Type = mProperties.Type;
			imageCreateInformation.Format = mProperties.Format;
			imageCreateInformation.FaceCount = mProperties.Type == TEX_TYPE_3D ? 1u : mProperties.GetFaceCount();
			imageCreateInformation.MipLevelCount = mipCount;
			imageCreateInformation.Usage = mProperties.Usage;
			imageCreateInformation.DebugName = GetName();

			MetalImage* image = mGpuDevice.GetResourceManager().Create<MetalImage>(imageCreateInformation, handle, allocation);

			if (!GetName().empty())
				image->SetName(GetName());

			return image;
			} // @autoreleasepool
		}

		GpuQueueMask MetalTexture::GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags) const
		{
			if (mImage == nullptr || mipLevel > mProperties.MipMapCount || arrayLayer >= (mProperties.Type == TEX_TYPE_3D ? 1u : mProperties.GetFaceCount()))
				return GpuQueueMask::kNone;

			return mImage->GetSubresourceUseInfo(arrayLayer, mipLevel, accessFlags);
		}

		u32 MetalTexture::GetBoundCount(u32 subresourceIdx) const
		{
			if (mImage == nullptr)
				return 0;

			u32 face, mipLevel;
			mProperties.MapFromSubresourceIndex(subresourceIdx, face, mipLevel);

			if (mProperties.Type == TEX_TYPE_3D)
				face = 0;

			return mImage->GetSubresourceBoundCount(face, mipLevel);
		}

		u32 MetalTexture::GetUseCount(u32 subresourceIdx) const
		{
			if (mImage == nullptr)
				return 0;

			u32 face, mipLevel;
			mProperties.MapFromSubresourceIndex(subresourceIdx, face, mipLevel);
			if (mProperties.Type == TEX_TYPE_3D)
				face = 0;

			return mImage->GetSubresourceUseCount(face, mipLevel);
		}

		GpuTextureMappedScope MetalTexture::Map(u32, u32, GpuMapOptions)
		{
			// Metal textures are always private-storage and therefore not directly mappable.
			return GpuTextureMappedScope();
		}

		void MetalTexture::Flush(u32, u32)
		{
			// No-op: private textures have no CPU-visible cache to flush.
		}

		void MetalTexture::Invalidate(u32, u32)
		{
			// No-op: private textures have no CPU-visible cache to invalidate.
		}
	} // namespace render
} // namespace b3d
