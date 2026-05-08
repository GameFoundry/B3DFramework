//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalTexture.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalUtility.h"
#include "Image/B3DPixelData.h"
#include "Debug/B3DLog.h"
#include "Utility/Threading/B3DThreading.h"

namespace b3d
{
	namespace render
	{
		struct MetalTexture::Impl
		{
			id<MTLTexture> Texture = nil;

			// Lazily-created MTLPixelFormat -> view cache, used for depth-stencil shader-read views.
			UnorderedMap<u32, id<MTLTexture>> ShaderReadViews;

			// Guards concurrent @c GetShaderReadView calls from multiple worker fibers populating the
			// cache for the same texture. @c newTextureViewWithPixelFormat: is invoked inside the lock
			// so two callers racing on a miss for the same format don't both pay the allocation cost
			// (or leave two live views for the same key).
			Mutex ViewCacheMutex;
		};

		MetalTexture::MetalTexture(MetalGpuDevice& gpuDevice, const TextureCreateInformation& createInformation)
			: Texture(createInformation), mGpuDevice(gpuDevice), mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalTexture::~MetalTexture()
		{
			// A'10: the MTLTexture (and any cached reinterpret views) may still be referenced by
			// in-flight command buffers when the last TShared<MetalTexture> is dropped. Mirror
			// @c RecreateInternalTexture's deferred-release pattern — queue the backing handles
			// against the graphics queue's last-committed watermark so they survive until any
			// in-flight command buffer that referenced them retires, then fall out of scope.
			//
			// Shortcuts:
			//   * No @c mImpl or no @c Texture → nothing to defer.
			//   * Device is shutting down (@c IsShuttingDown) → release synchronously. The device's
			//     @c ~MetalGpuDevice already drains @c DeferredReleases and then resets the heap
			//     allocator; queuing into the list after that drain would leave heap-backed entries
			//     holding parent-heap refs past @c mHeapAllocator.reset(). The shutdown path has
			//     already taken the @c WaitUntilIdle up front so nothing can still be in-flight.
			//   * No graphics queue resolvable, or @c lastCommitted == 0 → the resource couldn't
			//     have been scheduled, so immediate release is safe (symmetric with the recreate
			//     short-circuit).
			if (!mImpl || mImpl->Texture == nil)
			{
				ReleaseInternalTexture();
				return;
			}

			if (mGpuDevice.IsShuttingDown())
			{
				ReleaseInternalTexture();
				return;
			}

			TShared<GpuQueue> gfxQueue = mGpuDevice.GetQueue(GQT_GRAPHICS, 0);
			MetalGpuQueue* metalQueue = gfxQueue ? static_cast<MetalGpuQueue*>(gfxQueue.get()) : nullptr;
			const u64 lastCommitted = metalQueue != nullptr ? metalQueue->GetLastCommittedEventValue() : 0;

			if (metalQueue != nullptr && lastCommitted > 0)
			{
				// Transfer the strong refs into the deferred-release list. Move the local handles into
				// locals first so @c mImpl fields are nil'd synchronously — anyone still holding the
				// TShared (shouldn't happen, we're in the dtor) won't see a half-released state.
				id<MTLTexture> prior = mImpl->Texture;
				UnorderedMap<u32, id<MTLTexture>> priorViews;
				priorViews.swap(mImpl->ShaderReadViews);
				mImpl->Texture = nil;

				// Views reinterpret the parent's storage, so they must retire no later than the
				// parent. Queue each on the same watermark.
				for (auto& viewEntry : priorViews)
					mGpuDevice.QueueMetalResourceForDeferredRelease(viewEntry.second, metalQueue, lastCommitted);

				mGpuDevice.QueueMetalResourceForDeferredRelease(prior, metalQueue, lastCommitted);
				return;
			}

			ReleaseInternalTexture();
		}

		id<MTLTexture> MetalTexture::GetMetalTexture() const
		{
			return mImpl->Texture;
		}

		id<MTLTexture> MetalTexture::GetShaderReadView(MTLPixelFormat viewFormat)
		{
			if (mImpl->Texture == nil)
				return nil;

			if (viewFormat == [mImpl->Texture pixelFormat])
				return mImpl->Texture;

			// One fiber may be fetching a view while another adds to the same cache — serialize both
			// the @c find and the insert so the pair is atomic. View construction stays inside the
			// lock so two callers racing on a miss for the same format don't both pay the allocation
			// or leave two live MTLTextures mapped to one key.
			Lock lock(mImpl->ViewCacheMutex);

			const u32 key = (u32)viewFormat;
			auto existing = mImpl->ShaderReadViews.find(key);
			if (existing != mImpl->ShaderReadViews.end())
				return existing->second;

			const MTLPixelFormat parentFormat = [mImpl->Texture pixelFormat];

			// Combined depth-stencil textures need the 5-arg
			// @c newTextureViewWithPixelFormat:textureType:levels:slices: when reinterpreted as a
			// single-aspect view. The 1-arg form rejects DS-aspect splits because it cannot express
			// which plane (depth vs stencil) the view targets. For sRGB / linear reinterpretation —
			// or any other same-family case — keep the 1-arg form: it preserves texture type, level,
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
				// @c MipMapCount on the engine side is "additional mips beyond the base level", so the
				// total MTL level count is (MipMapCount + 1). Cube textures are stored in Metal as
				// 6*ArraySliceCount slices under @c MTLTextureTypeCube / CubeArray — the array-length
				// that @c newTextureViewWithPixelFormat: expects is the raw slice count, hence the *6.
				const u32 levelCount = mProperties.MipMapCount + 1;
				const u32 sliceCount = mProperties.ArraySliceCount
					* ((mProperties.Type == TEX_TYPE_CUBE_MAP) ? 6u : 1u);

				view = [mImpl->Texture newTextureViewWithPixelFormat:viewFormat
													  textureType:[mImpl->Texture textureType]
														   levels:NSMakeRange(0, levelCount)
														   slices:NSMakeRange(0, sliceCount)];
			}
			else
			{
				view = [mImpl->Texture newTextureViewWithPixelFormat:viewFormat];
			}

			if (view == nil)
			{
				B3D_LOG(Warning, LogRenderBackend,
					"Failed to create MTLTexture view for texture '{0}' with format {1}.",
					GetName(), (u32)viewFormat);
				return nil;
			}

			mImpl->ShaderReadViews[key] = view;
			return view;
		}

		void MetalTexture::SetName(const StringView& name)
		{
			// Delegate to the base so @c Texture::mName (read by @c GetName) is the single source of
			// truth — an earlier shadowing @c String mName here meant @c GetName returned an empty
			// string even after @c SetName ran.
			Texture::SetName(name);
			if (mImpl->Texture)
			{
				NSString* nsName = [NSString stringWithUTF8String:GetName().c_str()];
				[mImpl->Texture setLabel:nsName];
			}
		}

		void MetalTexture::Initialize()
		{
			CreateInternalTexture();

			// A'11: if the backing MTLTexture could not be allocated (unsupported format, heap-allocator
			// OOM, etc.) the downstream @c Texture::Initialize would still run @c TextureUtility::Write
			// against a nil target — the copy path bails early with a log but the engine observes a
			// "successful" init with no pixels uploaded and no error surfaced to the caller.
			// @c CreateInternalTexture has already logged the specific failure reason; we skip the base
			// upload path and make the failure visible to callers checking @c IsInitialized.
			if (!mImpl || mImpl->Texture == nil)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalTexture allocation failed; skipping pixel upload for texture '{0}'. Texture is unusable.",
					GetName());
				return;
			}

			// Delegate to @c Texture::Initialize so the base handles the @c mInitData pixel upload via
			// @c TextureUtility::Write. That path relies on the backend's @c MTLTexture already existing
			// and being named, which is why this call comes last. The base also unlocks @c mInitData and
			// invokes @c RenderProxy::Initialize — no explicit unlock or render-proxy init is needed here.
			Texture::Initialize();
		}

		void MetalTexture::RecreateInternalTexture()
		{
			// The prior MTLTexture may still be referenced by in-flight command buffers. Hand the
			// strong ref to the device's deferred-release list tagged with the graphics queue's
			// last-committed event value; the next @c BeginFrame drops it once that value has been
			// signaled. If the queue has never scheduled any work (ReservedValue == 0) the resource
			// cannot have been bound, so we short-circuit and let the local strong ref fall out of
			// scope, releasing immediately.
			//
			// TODO: this watermark covers only the graphics queue. If the texture was referenced solely
			// on the compute or transfer queue, the release may over-retain by up to one frame (the
			// graphics queue's frontier is usually ahead by that much). Tighten by recording the
			// resource's actual last-submit queue + value once per-resource submit tracking lands.
			id<MTLTexture> prior = mImpl->Texture;
			UnorderedMap<u32, id<MTLTexture>> priorViews;
			priorViews.swap(mImpl->ShaderReadViews);
			mImpl->Texture = nil;

			if (prior != nil)
			{
				TShared<GpuQueue> gfxQueue = mGpuDevice.GetQueue(GQT_GRAPHICS, 0);
				MetalGpuQueue* metalQueue = gfxQueue ? static_cast<MetalGpuQueue*>(gfxQueue.get()) : nullptr;
				const u64 lastCommitted = metalQueue != nullptr ? metalQueue->GetLastCommittedEventValue() : 0;

				if (metalQueue != nullptr && lastCommitted > 0)
				{
					// Views reinterpret the parent's storage, so they must retire no later than the
					// parent. Queue each on the same watermark.
					for (auto& viewEntry : priorViews)
						mGpuDevice.QueueMetalResourceForDeferredRelease(viewEntry.second, metalQueue, lastCommitted);

					mGpuDevice.QueueMetalResourceForDeferredRelease(prior, metalQueue, lastCommitted);
				}
				// else: the texture could not have been scheduled yet — strong refs fall out of scope here.
			}

			CreateInternalTexture();
		}

		void MetalTexture::CreateInternalTexture()
		{
			// Descriptor / NSString / texture allocations below are autoreleased; drain them locally
			// rather than relying on a runloop — there may be none under the engine's fiber scheduler.
			@autoreleasepool
			{
			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
				return;

			bool useSRGB = mProperties.UseHardwareSRGB;
			MTLPixelFormat mtlFormat = MetalUtility::GetPixelFormat(mProperties.Format, useSRGB);
			if (mtlFormat == MTLPixelFormatInvalid && useSRGB)
			{
				// Retry without sRGB; match the Vulkan backend's behavior where the linear variant is
				// used if the hardware cannot honor the sRGB request.
				B3D_LOG(Warning, LogRenderBackend,
					"MTLPixelFormat for format {0} unavailable in sRGB variant; falling back to linear.",
					(u32)mProperties.Format);
				useSRGB = false;
				mtlFormat = MetalUtility::GetPixelFormat(mProperties.Format, false);
			}
			if (mtlFormat == MTLPixelFormatInvalid)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create MTLTexture: unsupported pixel format {0}.", (u32)mProperties.Format);
				return;
			}

			// MSAA textures cannot have mip chains on Metal — descriptor validation rejects
			// @c sampleCount > 1 combined with @c mipmapLevelCount > 1. Force the mip count to 1 when
			// the caller asked for both; the engine's internal MSAA render-target path only ever needs
			// the base level anyway (downstream resolves happen into a separate, non-MSAA texture).
			u32 mipCount = mProperties.MipMapCount + 1;
			u32 sampleCount = std::max(1u, mProperties.SampleCount);

			// Not every sample count is available on every GPU (e.g. Apple-family typically maxes at
			// 4x; some desktop Intel GPUs historically skipped 8x). Probe and fall back to the nearest
			// lower supported count rather than letting the driver reject the descriptor outright.
			if (sampleCount > 1 && ![device supportsTextureSampleCount:sampleCount])
			{
				u32 fallback = 1;
				for (u32 probe = sampleCount - 1; probe >= 1; --probe)
				{
					if ([device supportsTextureSampleCount:probe])
					{
						fallback = probe;
						break;
					}
					if (probe == 1)
						break;
				}
				B3D_LOG(Warning, LogRenderBackend,
					"MTLDevice does not support sampleCount={0}; falling back to {1}.",
					sampleCount, fallback);
				sampleCount = fallback;
			}

			if (sampleCount > 1 && mipCount > 1)
			{
				B3D_LOG(Warning, LogRenderBackend,
					"MTLTexture requested with sampleCount={0} and mipmapLevelCount={1}; Metal rejects MSAA + mips, forcing mipmapLevelCount=1.",
					sampleCount, mipCount);
				mipCount = 1;
			}

			MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
			desc.textureType = MetalUtility::GetTextureType(mProperties.Type, sampleCount, mProperties.ArraySliceCount);
			desc.pixelFormat = mtlFormat;
			desc.width = mProperties.Width;
			desc.height = mProperties.Type == TEX_TYPE_1D ? 1 : mProperties.Height;
			desc.depth = mProperties.Type == TEX_TYPE_3D ? mProperties.Depth : 1;
			desc.mipmapLevelCount = mipCount;
			desc.sampleCount = sampleCount;
			// For cube maps Metal's arrayLength is the number of cube sets (faces = 6 * arrayLength),
			// so propagate ArraySliceCount directly. Only TEX_TYPE_3D is non-array in Metal.
			desc.arrayLength = (mProperties.Type == TEX_TYPE_3D) ? 1 : mProperties.ArraySliceCount;

			// Map engine usage flags to Metal usage flags. @c MTLTextureUsagePixelFormatView is set
			// unconditionally: @c [MTLTexture newTextureViewWithPixelFormat:] (used by
			// @c GetShaderReadView) silently returns nil if the source texture wasn't created with it,
			// and the sRGB/linear, depth-plane, and LoadStore-reinterpretation paths all need views
			// over regular shader-read textures too. Cost is per-format metadata at creation — no
			// runtime overhead for textures that never get a view.
			MTLTextureUsage usage = MTLTextureUsageShaderRead | MTLTextureUsagePixelFormatView;
			if (mProperties.Usage & TextureUsageFlag::RenderTarget)
				usage |= MTLTextureUsageRenderTarget;
			if (mProperties.Usage & TextureUsageFlag::DepthStencil)
				usage |= MTLTextureUsageRenderTarget;
			if (mProperties.Usage & TextureUsageFlag::LoadStore)
				usage |= MTLTextureUsageShaderWrite;
			desc.usage = usage;

			// Textures always use private storage. CPU traffic runs through TextureUtility::Write /
			// TextureUtility::Read, which stage into a GpuBuffer and then drive CopyBufferToTexture /
			// CopyTextureToBuffer on the command buffer. Direct Map on Metal textures is out of scope
			// for phase 2 (see Texture::Map contract in B3DTexture.h).
			desc.storageMode = MTLStorageModePrivate;

			// TODO: phase-2 review S37 — DEFERRED. Leave @c hazardTrackingMode at its default
			// (→ Tracked on macOS) so the driver auto-handles read-after-write / write-after-write
			// hazards for directly-bound resources *and* for argument-buffer-referenced resources
			// reached through @c useResource:usage:stages:. Flipping this to @c Untracked is a perf
			// optimization that requires an engine-side resource tracker (S14) to land first; see the
			// matching TODO in @c B3DMetalGpuCommandBuffer.mm::IssueBarriers.

			// Route through the device's heap allocator so the common case (textures under the
			// large-resource threshold, private or shared storage) sub-allocates out of a pooled
			// MTLHeap rather than paying the per-resource driver-side allocation cost. Oversized
			// render targets / streaming textures fall back to direct device allocation inside the
			// allocator; nothing in this method needs to know which path was taken.
			mImpl->Texture = mGpuDevice.GetHeapAllocator().AllocateTexture(desc);
#if !__has_feature(objc_arc)
			[desc release];
#endif

			if (mImpl->Texture == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create MTLTexture (format {0}, {1}x{2}).",
					(u32)mProperties.Format, mProperties.Width, mProperties.Height);
				return;
			}

			if (!GetName().empty())
			{
				NSString* nsName = [NSString stringWithUTF8String:GetName().c_str()];
				[mImpl->Texture setLabel:nsName];
			}
			} // @autoreleasepool
		}

		void MetalTexture::ReleaseInternalTexture()
		{
			if (!mImpl)
				return;

			mImpl->Texture = nil;
			mImpl->ShaderReadViews.clear();
		}

		GpuTextureMappedScope MetalTexture::Map(u32, u32, GpuMapOptions)
		{
			// Metal textures are always private-storage in phase 2 and therefore not directly
			// mappable. Callers should use TextureUtility::Write / TextureUtility::Read, which stage
			// through a GpuBuffer and drive CopyBufferToTexture / CopyTextureToBuffer on the command
			// buffer. The invalid scope returned here is the engine contract's signal that the caller
			// must take the staging path.
			return GpuTextureMappedScope();
		}

		void MetalTexture::Flush(u32, u32)
		{
			// No-op: private textures have no CPU-visible cache to flush. Present to satisfy the
			// pure-virtual Texture::Flush override.
		}

	} // namespace render
} // namespace b3d
