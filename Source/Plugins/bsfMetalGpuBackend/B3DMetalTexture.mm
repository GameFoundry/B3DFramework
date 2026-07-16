//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalTexture.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalUtility.h"
#include "Debug/B3DLog.h"

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

		MetalImageSubresource::MetalImageSubresource(MetalResourceManager* owner, GpuImageLayout layout, const StringView& name)
			: MetalResource(owner, name), mLayout(layout)
		{ }

		MetalImage::MetalImage(MetalResourceManager* owner, const MetalImageCreateInformation& createInformation, MetalTextureNativeHandle texture, const GpuResourceLocation& allocation)
			: TMetalResource<IGpuImageResource>(owner, createInformation.DebugName, createInformation.FaceCount, createInformation.MipLevelCount, GetFullAspectFlags(createInformation.Usage, createInformation.Format))
			, mTexture(texture)
			, mAllocation(allocation)
			, mType(createInformation.Type)
		{
			// Fill in the per-(face x mip) subresource wrappers the IGpuImageResource base
			// allocated (zero-initialized). The resource tracker uses these to track subresource
			// usage individually. Fresh image contents are undefined, mirroring Vulkan's
			// VK_IMAGE_LAYOUT_UNDEFINED starting state.
			const u32 subresourceCount = mFaceCount * mMipLevelCount;
			for (u32 subresourceIndex = 0; subresourceIndex < subresourceCount; subresourceIndex++)
				mSubresources[subresourceIndex] = owner->Create<MetalImageSubresource>(GpuImageLayout::Undefined);
		}

		MetalImage::~MetalImage()
		{
			const u32 subresourceCount = mFaceCount * mMipLevelCount;
			for (u32 subresourceIndex = 0; subresourceIndex < subresourceCount; subresourceIndex++)
			{
				B3D_ASSERT(!mSubresources[subresourceIndex]->IsBound()); // Image being freed but its subresources are still bound somewhere

				mSubresources[subresourceIndex]->Destroy();
			}

			// Views reinterpret this image's storage, so they must be released no later than the
			// parent handle. The manager's deferred-destroy path guarantees this destructor only
			// runs once no command buffer references the image, so synchronous release is safe.
			{
				Lock lock(mViewCacheMutex);
#if !__has_feature(objc_arc)
				for (auto& viewEntry : mShaderReadViews)
					[viewEntry.second release];
#endif
				mShaderReadViews.clear();
			}

#if !__has_feature(objc_arc)
			[mTexture release];
#endif
			mTexture = nullptr;

			// Allocator-backed spans return to the device's persistent TLSF pool; direct device
			// allocations carry an invalid location. ResourceLifecycle deferral mode reclaims
			// immediately.
			if (mAllocation.IsValid())
				mAllocation.Allocator->Free(mAllocation);
		}

		void MetalImage::SetName(const StringView& name)
		{
			if (mTexture == nullptr)
				return;

			// NSString below is autoreleased; drain locally — there may be no runloop under the
			// engine's fiber scheduler. StringView is not guaranteed null-terminated, so copy first.
			@autoreleasepool
			{
				const String nameCopy(name.data(), name.size());
				[mTexture setLabel:[NSString stringWithUTF8String:nameCopy.c_str()]];
			}
		}

		MetalImageSubresource* MetalImage::GetSubresource(u32 face, u32 mipLevel)
		{
			return static_cast<MetalImageSubresource*>(IGpuImageResource::GetSubresource(face, mipLevel));
		}

		id<MTLTexture> MetalImage::GetShaderReadView(MTLPixelFormat viewFormat)
		{
			if (mTexture == nil)
				return nil;

			if (viewFormat == [mTexture pixelFormat])
				return mTexture;

			// One fiber may be fetching a view while another adds to the same cache — serialize
			// both the find and the insert so the pair is atomic (see mViewCacheMutex docs).
			Lock lock(mViewCacheMutex);

			const u32 key = (u32)viewFormat;
			auto existing = mShaderReadViews.find(key);
			if (existing != mShaderReadViews.end())
				return existing->second;

			const MTLPixelFormat parentFormat = [mTexture pixelFormat];

			// Combined depth-stencil textures need the 4-argument
			// newTextureViewWithPixelFormat:textureType:levels:slices: when reinterpreted as a
			// single-aspect view — the 1-argument form rejects DS-aspect splits because it cannot
			// express which plane the view targets. For sRGB / linear reinterpretation (or any
			// other same-family case) keep the 1-argument form: it preserves texture type, level
			// and slice ranges implicitly and is cheaper at creation.
			const bool parentIsCombinedDS = (parentFormat == MTLPixelFormatDepth32Float_Stencil8)
				|| (parentFormat == MTLPixelFormatDepth24Unorm_Stencil8);
			const bool viewIsSingleAspect = (viewFormat == MTLPixelFormatDepth32Float)
				|| (viewFormat == MTLPixelFormatDepth16Unorm)
				|| (viewFormat == MTLPixelFormatStencil8)
				|| (viewFormat == MTLPixelFormatX24_Stencil8)
				|| (viewFormat == MTLPixelFormatX32_Stencil8);

			id<MTLTexture> view = nil;
			if (parentIsCombinedDS && viewIsSingleAspect)
			{
				// Metal stores cube textures as 6 * arrayLength slices under
				// MTLTextureTypeCube / CubeArray — the slice range the view initializer expects is
				// the raw slice count, hence the *6. Level/slice counts are queried off the native
				// texture so the view always covers the full resource.
				const MTLTextureType textureType = [mTexture textureType];
				const bool isCube = textureType == MTLTextureTypeCube || textureType == MTLTextureTypeCubeArray;
				const NSUInteger sliceCount = [mTexture arrayLength] * (isCube ? 6 : 1);

				view = [mTexture newTextureViewWithPixelFormat:viewFormat
												   textureType:textureType
														levels:NSMakeRange(0, [mTexture mipmapLevelCount])
														slices:NSMakeRange(0, sliceCount)];
			}
			else
			{
				view = [mTexture newTextureViewWithPixelFormat:viewFormat];
			}

			if (view == nil)
			{
				B3D_LOG(Warning, LogRenderBackend,
					"Failed to create MTLTexture view with format {0}.", (u32)viewFormat);
				return nil;
			}

			mShaderReadViews[key] = view;
			return view;
		}

		MetalTexture::MetalTexture(MetalGpuDevice& gpuDevice, const TextureCreateInformation& createInformation)
			: Texture(createInformation), mGpuDevice(gpuDevice)
		{ }

		MetalTexture::~MetalTexture()
		{
			// Queue the wrapper (native texture + cached reinterpret views) for destruction; the
			// manager defers the actual release until every command buffer referencing it retires.
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

		void MetalTexture::SetName(const StringView& name)
		{
			// Delegate to the base so Texture::mName (read by GetName) is the single source of truth.
			Texture::SetName(name);

			if (mImage != nullptr)
				mImage->SetName(name);
		}

		void MetalTexture::Initialize()
		{
			mImage = CreateImage();

			// If the backing MTLTexture could not be allocated (unsupported format, allocator OOM,
			// etc.) the downstream Texture::Initialize would still run TextureUtility::Write
			// against a nil target — the copy path bails early with a log but the engine would
			// observe a "successful" init with no pixels uploaded. CreateImage has already logged
			// the specific failure reason; skip the base upload path and make the failure visible
			// to callers checking IsInitialized.
			if (mImage == nullptr)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalTexture allocation failed; skipping pixel upload for texture '{0}'. Texture is unusable.",
					GetName());
				return;
			}

			// Delegate to Texture::Initialize so the base handles the mInitData pixel upload via
			// TextureUtility::Write. That path relies on the backend's MTLTexture already existing
			// and being named, which is why this call comes last. The base also unlocks mInitData
			// and invokes RenderProxy::Initialize — no explicit unlock or render-proxy init is
			// needed here.
			Texture::Initialize();
		}

		void MetalTexture::RecreateInternalTexture()
		{
			MetalImage* newImage = CreateImage();

			// Queue the previous wrapper for destruction. The manager defers the release until
			// every command buffer referencing the image (or its cached reinterpret views, which
			// the wrapper owns) has retired, so in-flight GPU work keeps reading the old MTLTexture
			// safely while new writes target the fresh one.
			if (mImage != nullptr)
				mImage->Destroy();

			mImage = newImage;
		}

		MetalImage* MetalTexture::CreateImage()
		{
			// Descriptor / NSString / texture allocations below are autoreleased; drain them
			// locally rather than relying on a runloop — there may be none under the engine's
			// fiber scheduler.
			@autoreleasepool
			{
			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
				return nullptr;

			bool useSRGB = mProperties.UseHardwareSRGB;
			MTLPixelFormat mtlFormat = MetalUtility::GetPixelFormat(mProperties.Format, useSRGB);
			if (mtlFormat == MTLPixelFormatInvalid && useSRGB)
			{
				// Retry without sRGB; match the Vulkan backend's behavior where the linear variant
				// is used if the hardware cannot honor the sRGB request.
				B3D_LOG(Warning, LogRenderBackend,
					"MTLPixelFormat for format {0} unavailable in sRGB variant; falling back to linear.",
					(u32)mProperties.Format);
				useSRGB = false;
				mtlFormat = MetalUtility::GetPixelFormat(mProperties.Format, false);
			}
			if (mtlFormat == MTLPixelFormatInvalid)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create MTLTexture: unsupported pixel format {0}.", (u32)mProperties.Format);
				return nullptr;
			}

			// MSAA textures cannot have mip chains on Metal — descriptor validation rejects
			// sampleCount > 1 combined with mipmapLevelCount > 1. Fail unsupported combinations so
			// engine-visible properties remain identical to the native resource.
			bool supportsBCTextureCompression = false;
			if (@available(macOS 11.0, *))
				supportsBCTextureCompression = [device supportsBCTextureCompression];
			if (PixelUtility::IsCompressed(mProperties.Format) && !supportsBCTextureCompression)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Cannot create compressed MTLTexture: this Apple GPU does not support BC texture compression.");
				return nullptr;
			}

			if (mProperties.Width == 0 || (mProperties.Type != TEX_TYPE_1D && mProperties.Height == 0) ||
				(mProperties.Type == TEX_TYPE_3D && mProperties.Depth == 0))
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create an MTLTexture with a zero relevant dimension.");
				return nullptr;
			}

			const u32 width = mProperties.Width;
			const u32 height = mProperties.Type == TEX_TYPE_1D ? 1u : mProperties.Height;
			const u32 depth = mProperties.Type == TEX_TYPE_3D ? mProperties.Depth : 1u;
			const u32 mipCount = mProperties.MipMapCount + 1;
			const u32 sampleCount = std::max(1u, mProperties.SampleCount);

			if (mProperties.ArraySliceCount == 0)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create MTLTexture with zero array slices.");
				return nullptr;
			}

			if ((mProperties.Type == TEX_TYPE_1D && (mProperties.Height > 1 || mProperties.Depth > 1)) ||
				((mProperties.Type == TEX_TYPE_2D || mProperties.Type == TEX_TYPE_CUBE_MAP) && mProperties.Depth > 1))
			{
				B3D_LOG(Error, LogRenderBackend, "MTLTexture dimensions do not match the requested texture type.");
				return nullptr;
			}

			if (mProperties.Type == TEX_TYPE_CUBE_MAP && width != height)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create a non-square Metal cube texture ({0}x{1}).", width, height);
				return nullptr;
			}

			u32 maximumMipCount = 1;
			for (u32 maximumDimension = std::max(width, std::max(height, depth)); maximumDimension > 1; maximumDimension >>= 1)
				maximumMipCount++;
			if (mipCount > maximumMipCount)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Cannot create MTLTexture with {0} mip levels; dimensions allow at most {1}.", mipCount, maximumMipCount);
				return nullptr;
			}

			if (sampleCount > 1 && ![device supportsTextureSampleCount:sampleCount])
			{
				B3D_LOG(Error, LogRenderBackend, "MTLDevice does not support texture sample count {0}.", sampleCount);
				return nullptr;
			}

			if (sampleCount > 1 && mipCount > 1)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal does not support multisampled textures with mipmaps.");
				return nullptr;
			}

			if (sampleCount > 1 && mProperties.Type != TEX_TYPE_2D)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal multisampling is supported only for 2D textures.");
				return nullptr;
			}

			MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
			desc.textureType = MetalUtility::GetTextureType(mProperties.Type, sampleCount, mProperties.ArraySliceCount);
			desc.pixelFormat = mtlFormat;
			desc.width = width;
			desc.height = height;
			desc.depth = depth;
			desc.mipmapLevelCount = mipCount;
			desc.sampleCount = sampleCount;
			// For cube maps Metal's arrayLength is the number of cube sets (faces = 6 * arrayLength),
			// so propagate ArraySliceCount directly. Only TEX_TYPE_3D is non-array in Metal.
			desc.arrayLength = (mProperties.Type == TEX_TYPE_3D) ? 1 : mProperties.ArraySliceCount;

			// Map engine usage flags to Metal usage flags. MTLTextureUsagePixelFormatView is set
			// only for explicit mutable resources and depth-stencil plane views. Linear/sRGB views
			// do not require it, allowing immutable textures to retain the optimal native layout.
			MTLTextureUsage usage = MTLTextureUsageShaderRead;
			if (mProperties.Usage.IsSet(TextureUsageFlag::MutableFormat) || mProperties.Format == PF_D32_S8X24)
				usage |= MTLTextureUsagePixelFormatView;
			if (mProperties.Usage & TextureUsageFlag::RenderTarget)
				usage |= MTLTextureUsageRenderTarget;
			if (mProperties.Usage & TextureUsageFlag::DepthStencil)
				usage |= MTLTextureUsageRenderTarget;
			if (mProperties.Usage & TextureUsageFlag::AllowUnorderedAccessOnTheGPU)
				usage |= MTLTextureUsageShaderWrite;
			desc.usage = usage;

			// Textures always use private storage. CPU traffic runs through
			// TextureUtility::Write / TextureUtility::Read, which stage into a GpuBuffer and then
			// drive CopyBufferToTexture / CopyTextureToBuffer on the command buffer. Direct Map on
			// Metal textures is out of scope (see Texture::Map contract in B3DTexture.h).
			desc.storageMode = MTLStorageModePrivate;

			// Direct textures and placement-heap textures use the same configured hazard policy.
			if (@available(macOS 10.15, iOS 13.0, *))
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				desc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
#else
				desc.hazardTrackingMode = MTLHazardTrackingModeTracked;
#endif
			}

			// Route through the device's memory manager so the texture sub-allocates out of a
			// pooled placement MTLHeap at an allocator-chosen offset rather than paying the
			// per-resource driver-side allocation cost. Oversized or non-poolable requests fall
			// back to direct device allocation inside the allocator; the invalid location tells the
			// wrapper nothing needs freeing back to the pool.
			GpuResourceLocation location;
			MetalTextureNativeHandle handle = mGpuDevice.GetHeapAllocator().AllocateTexture(desc, location);
#if !__has_feature(objc_arc)
			[desc release];
#endif

			if (handle == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create MTLTexture (format {0}, {1}x{2}).",
					(u32)mProperties.Format, mProperties.Width, mProperties.Height);
				return nullptr;
			}

			mInternalFormat = mProperties.Format;

			MetalImageCreateInformation imageCreateInformation;
			imageCreateInformation.Type = mProperties.Type;
			imageCreateInformation.Format = mProperties.Format;
			imageCreateInformation.FaceCount = mProperties.Type == TEX_TYPE_3D ? 1u : mProperties.GetFaceCount();
			imageCreateInformation.MipLevelCount = mipCount;
			imageCreateInformation.Usage = mProperties.Usage;
			imageCreateInformation.DebugName = GetName();

			MetalImage* image = mGpuDevice.GetResourceManager().Create<MetalImage>(imageCreateInformation, handle, location);

			if (!GetName().empty())
				image->SetName(GetName());

			return image;
			} // @autoreleasepool
		}

		GpuQueueMask MetalTexture::GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags) const
		{
			if (mImage == nullptr || mipLevel > mProperties.MipMapCount ||
				arrayLayer >= (mProperties.Type == TEX_TYPE_3D ? 1u : mProperties.GetFaceCount()))
				return GpuQueueMask::kNone;

			return mImage->GetSubresource(arrayLayer, mipLevel)->GetUseInfo(accessFlags);
		}

		u32 MetalTexture::GetBoundCount(u32 subresourceIdx) const
		{
			if (mImage == nullptr)
				return 0;

			u32 face, mipLevel;
			mProperties.MapFromSubresourceIndex(subresourceIdx, face, mipLevel);
			if (mProperties.Type == TEX_TYPE_3D)
				face = 0;

			return mImage->GetSubresource(face, mipLevel)->GetBoundCount();
		}

		u32 MetalTexture::GetUseCount(u32 subresourceIdx) const
		{
			if (mImage == nullptr)
				return 0;

			u32 face, mipLevel;
			mProperties.MapFromSubresourceIndex(subresourceIdx, face, mipLevel);
			if (mProperties.Type == TEX_TYPE_3D)
				face = 0;

			return mImage->GetSubresource(face, mipLevel)->GetUseCount();
		}

		GpuTextureMappedScope MetalTexture::Map(u32, u32, GpuMapOptions)
		{
			// Metal textures are always private-storage and therefore not directly mappable.
			// Callers should use TextureUtility::Write / TextureUtility::Read, which stage through
			// a GpuBuffer and drive CopyBufferToTexture / CopyTextureToBuffer on the command
			// buffer. The invalid scope returned here is the engine contract's signal that the
			// caller must take the staging path.
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
