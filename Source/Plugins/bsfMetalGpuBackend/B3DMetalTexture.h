//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalResource.h"
#include "GpuBackend/B3DGpuCommandBuffer.h" // GpuImageLayout
#include "Image/B3DTexture.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

#ifdef __OBJC__
		/** Native Metal texture handle. Aliased to void* in plain C++ TUs so class layouts stay identical (id is a pointer). */
		using MetalTextureNativeHandle = id<MTLTexture>;
#else
		using MetalTextureNativeHandle = void*;
#endif

		class MetalTexture;

		/** Descriptor used for initializing a MetalImage. */
		struct MetalImageCreateInformation
		{
			TextureType Type = TEX_TYPE_2D; /**< Type of the image. */
			PixelFormat Format = PF_UNKNOWN; /**< Engine pixel format of the image (used for aspect deduction). */
			u32 FaceCount = 1; /**< Number of faces (array slices; 6 per slice for cube maps). */
			u32 MipLevelCount = 1; /**< Number of mipmap levels per face. */
			TextureUsageFlags Usage; /**< Determines how the image will be used. */
			StringView DebugName; /**< Optional name of the resource, for debugging purposes. */
		};

		/** Represents a single sub-resource (face & mip level) of a larger image object. */
		class MetalImageSubresource : public MetalResource
		{
		public:
			MetalImageSubresource(MetalResourceManager* owner, GpuImageLayout layout, const StringView& name = "");

			/**
			 * Returns the layout the subresource is currently in. Metal has no native image
			 * layouts; this is engine-side bookkeeping used to communicate tracked layout state
			 * between command buffers, updated only after command buffer submit.
			 *
			 * @note	Submit thread only.
			 */
			GpuImageLayout GetLayout() const { return mLayout; }

			/**
			 * Notifies the resource that the tracked subresource layout has changed.
			 *
			 * @note	Submit thread only.
			 */
			void SetLayout(GpuImageLayout layout) { mLayout = layout; }

		private:
			GpuImageLayout mLayout;
		};

		/** Wrapper around a Metal texture object that manages its usage and lifetime. */
		class MetalImage : public TMetalResource<IGpuImageResource>
		{
		public:
			/**
			 * @param	owner				Resource manager that keeps track of lifetime of this resource.
			 * @param	createInformation	Describes the image being wrapped.
			 * @param	texture				Native MTLTexture handle the wrapper takes ownership of (+1 reference under MRC).
			 * @param	allocation			Engine allocator span backing the image's memory, or an invalid location for
			 *								direct (non-sub-allocated) device allocations.
			 */
			MetalImage(MetalResourceManager* owner, const MetalImageCreateInformation& createInformation, MetalTextureNativeHandle texture, const GpuResourceLocation& allocation);
			~MetalImage();

			/** Assigns a name to the image, primarily used for easier debugging. */
			void SetName(const StringView& name);

			/**
			 * Retrieves a separate resource for a specific image face & mip level. This allows the
			 * caller to track subresource usage individually, instead of for the entire image.
			 */
			MetalImageSubresource* GetSubresource(u32 face, u32 mipLevel);

#ifdef __OBJC__
			/** Returns the internal handle to the Metal object. */
			id<MTLTexture> GetMetalHandle() const { return mTexture; }

			/**
			 * Returns a lazily-created @c MTLTexture view that reinterprets the storage with a
			 * different pixel format. Typical use is sampling the depth plane of a combined
			 * depth-stencil texture from a shader. Views are cached for the lifetime of the image
			 * and retire with it.
			 */
			id<MTLTexture> GetShaderReadView(MTLPixelFormat viewFormat);
#endif

		private:
			MetalTextureNativeHandle mTexture = nullptr;
			GpuResourceLocation mAllocation;
			TextureType mType = TEX_TYPE_2D;

			/**
			 * Lazily-created MTLPixelFormat -> view cache, used for depth-stencil / sRGB shader-read
			 * views. Values are +1 retained references (MRC), released in the destructor.
			 */
			UnorderedMap<u32, MetalTextureNativeHandle> mShaderReadViews;

			/**
			 * Guards concurrent GetShaderReadView calls from multiple worker fibers populating the
			 * cache for the same image. View construction stays inside the lock so two callers
			 * racing on a miss for the same format don't both pay the allocation cost (or leave two
			 * live views for the same key).
			 */
			mutable Mutex mViewCacheMutex;
		};

		/**
		 * Metal implementation of a texture.
		 *
		 * High-level proxy over a private-storage MetalImage wrapper. Metal textures are not
		 * directly mappable, so @c Map returns an invalid @c GpuTextureMappedScope and callers are
		 * expected to route CPU traffic through @c TextureUtility::Write / @c TextureUtility::Read,
		 * which drives @c CopyBufferToTexture / @c CopyTextureToBuffer on the command buffer.
		 */
		class MetalTexture : public Texture
		{
		public:
			MetalTexture(MetalGpuDevice& gpuDevice, const TextureCreateInformation& createInformation);
			~MetalTexture();

			void SetName(const StringView& name) override;
			GpuDevice& GetDevice() const override { return mGpuDevice; }
			PixelFormat GetSupportedFormat() const override { return mInternalFormat; }
			GpuQueueMask GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags = GpuAccessFlag::Read | GpuAccessFlag::Write) const override;
			u32 GetBoundCount(u32 subresourceIdx = 0) const override;
			u32 GetUseCount(u32 subresourceIdx = 0) const override;
			void Flush(u32 mipLevel, u32 arrayLayer) override;
			void Invalidate(u32 mipLevel, u32 arrayLayer) override;

			/** Gets the resource wrapping the Metal texture object. */
			MetalImage* GetMetalResource() const { return mImage; }

#ifdef __OBJC__
			/** Returns the underlying MTLTexture. May be nil if Initialize() failed or has not been called yet. */
			id<MTLTexture> GetMetalTexture() const;

			/** @copydoc MetalImage::GetShaderReadView */
			id<MTLTexture> GetShaderReadView(MTLPixelFormat viewFormat);
#endif

		protected:
			friend class MetalGpuDevice;
			friend class MetalImage;

			void Initialize() override;
			void RecreateInternalTexture() override;
			GpuTextureMappedScope Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options) override;

		private:
			/** Creates a new image wrapper matching the current texture properties. Returns null on failure. */
			MetalImage* CreateImage();

			MetalGpuDevice& mGpuDevice;
			MetalImage* mImage = nullptr;
			PixelFormat mInternalFormat = PF_UNKNOWN;
		};

		/** @} */
	} // namespace render
} // namespace b3d
