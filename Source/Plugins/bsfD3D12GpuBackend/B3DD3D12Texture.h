//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12TextureLayout.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "B3DD3D12Resource.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Represents a single subresource (face × mip) of a D3D12Image, so per-subresource usage can be tracked
		 * individually by the resource tracker. Native submission state is isolated from command-buffer-local tracking.
		 */
		class D3D12ImageSubresource : public D3D12Resource
		{
		public:
			/** Creates submission state initialized to @p layout. */
			D3D12ImageSubresource(D3D12ResourceManager* owner, const D3D12TextureLayout& layout, const StringView& name = "");

			/**
			 * Returns the native layout committed by the most recently submitted command buffer.
			 *
			 * @note Submit thread only.
			 */
			const D3D12TextureLayout& GetLayout() const;

			/** 
			 * Updates the native layout after the command buffer that used this subresource has been submitted.
			 *
			 * @note Submit thread only. 
			 */
			void SetLayout(const D3D12TextureLayout& layout);

			/** 
			 * Returns the queue capable of transitioning the committed layout, if one exists. 
			 *
			 * @note Submit thread only. 
			 */
			bool GetLayoutTransitionQueueId(GpuQueueId& outQueueId) const;

			/** 
			 * Sets the queue that most recently transitioned the committed layout. 
			 * 
			 * @note Submit thread only. 
			 */
			void SetLayoutTransitionQueueId(GpuQueueId queueId);

		private:
			D3D12TextureLayout mLayout;
			GpuQueueId mLayoutTransitionQueueId;
			bool mHasLayoutTransitionQueue = false;
		};

		/** Descriptor structure used for initialization of a D3D12Image. */
		struct D3D12ImageCreateInformation
		{
			ComPtr<ID3D12Resource> Resource; /**< Native resource wrapped by the image. */
			GpuResourceLocation Allocation; /**< Memory allocation backing the resource. Invalid for externally owned resources (swap-chain buffers). */
			DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN; /**< Format of the resource. */
			D3D12TextureLayout InitialLayout; /**< Native layout the resource was created or acquired in for every plane. */
			u32 FaceCount = 1; /**< Number of array slices (or cube faces) in the image. */
			u32 MipLevelCount = 1; /**< Number of mip levels in the image. */
			GpuTextureAspectFlags Aspect = GpuTextureAspectFlag::Color; /**< Which aspects (color/depth/stencil) the image format contains. */
			bool AllowConcurrentQueueReads = false; /**< Whether shader reads may overlap on multiple GPU queues. */
			String Name; /**< Optional debug name. */
		};

		/**
		 * Wraps a native D3D12 texture resource and its memory allocation. Lifetime is owned by the device's
		 * resource manager and released via IGpuResource::Destroy(), deferred until the GPU is done with the
		 * resource. Owns one D3D12ImageSubresource per (face × mip) for per-subresource usage/state tracking.
		 */
		class D3D12Image : public TD3D12Resource<IGpuImageResource>
		{
		public:
			D3D12Image(D3D12ResourceManager* owner, const D3D12ImageCreateInformation& createInformation);
			~D3D12Image() override;

			/** Returns the native D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const { return mResource.Get(); }

			/** Returns the DXGI format of the image. */
			DXGI_FORMAT GetDXGIFormat() const { return mFormat; }

			/** Returns whether shader reads may overlap on multiple GPU queues. */
			bool AllowsConcurrentQueueReads() const { return mAllowConcurrentQueueReads; }

			using IGpuImageResource::GetRange;

			/** Builds the subresource range selected by @p surface (its face/mip window), clamped to the image. */
			GpuTextureSubresourceRange GetRange(const TextureSurface& surface) const;

			/** Returns the typed subresource object for the specified face and mip level. */
			D3D12ImageSubresource* GetD3D12Subresource(u32 face, u32 mipLevel) const
			{
				return static_cast<D3D12ImageSubresource*>(GetSubresource(face, mipLevel));
			}

			/** Returns the D3D12 subresource index (mip-major, as used by native transition barriers) for a face/mip pair. */
			u32 GetNativeSubresourceIndex(u32 face, u32 mipLevel) const { return face * mMipLevelCount + mipLevel; }

		private:
			ComPtr<ID3D12Resource> mResource;
			GpuResourceLocation mAllocation;
			DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
			bool mAllowConcurrentQueueReads = false;
		};

		/** DirectX 12 implementation of a texture. */
		class D3D12Texture : public Texture
		{
		public:
			/** Creates a texture owned by @p device. Call Initialize() before use. */
			D3D12Texture(const TextureCreateInformation& createInformation, GpuDevice& device);
			~D3D12Texture() override;

			void Initialize() override;
			GpuTextureMappedScope Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options) override;
			void RecreateInternalTexture() override;
			ImageSubresourcePitch GetStagingBufferPitchForSubresource(u32 face, u32 mipLevel) const override;
			GpuQueueMask GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags = GpuAccessFlag::Read | GpuAccessFlag::Write) const override;
			u32 GetBoundCount(u32 subresourceIndex = 0) const override;
			u32 GetUseCount(u32 subresourceIndex = 0) const override;

			/** Returns the low-level image resource wrapping the native D3D12 texture. */
			D3D12Image* GetD3D12Image() const { return mImage; }

			/** Returns the native D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const { return mImage != nullptr ? mImage->GetD3D12Resource() : nullptr; }

			/** Returns the DXGI format of the texture. */
			DXGI_FORMAT GetDXGIFormat() const { return mDXGIFormat; }

			GpuDevice& GetDevice() const override { return mGpuDevice; }

			/**
			 * Byte pitch between rows that staging buffers use for copies to or from the given subresource. Padded to
			 * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT as required for placed copy footprints, while remaining a whole
			 * number of format blocks.
			 */
			u32 GetStagingRowPitchInBytes(u32 mipLevel) const;

			/**
			 * Returns a CPU descriptor handle for a shader resource view (SRV) covering the specified surface. Views are
			 * cached and reused for identical surface requests. Returns a zeroed handle if the view could not be created.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle(const TextureSurface& surface);

			/**
			 * Returns a CPU descriptor handle for an unordered access view (UAV) covering the specified surface. Only
			 * valid for textures created with AllowUnorderedAccessOnTheGPU. Returns a zeroed handle otherwise.
			 */
			D3D12_CPU_DESCRIPTOR_HANDLE GetUAVHandle(const TextureSurface& surface);

		private:
			/** Distinguishes cached view types. */
			enum class ViewType
			{
				SRV, /**< Shader resource view. */
				UAV  /**< Unordered access view. */
			};

			/** Cached texture view. */
			struct ViewEntry
			{
				/** Creates a cached view for @p surface and @p type using @p handle. */
				ViewEntry(const TextureSurface& surface, ViewType type, D3D12_CPU_DESCRIPTOR_HANDLE handle)
					: Surface(surface), Type(type), Handle(handle)
				{ }

				TextureSurface Surface; /**< Texture surface covered by the view. */
				ViewType Type;          /**< Native descriptor type. */
				D3D12_CPU_DESCRIPTOR_HANDLE Handle; /**< CPU descriptor allocated for the view. */
			};

			/** Creates the D3D12 texture resource and its D3D12Image wrapper. */
			void CreateTexture();

			/** Queues the current D3D12Image (if any) for deferred destruction and drops all cached views. */
			void ReleaseTexture();

			/** Frees all cached view descriptors. */
			void ReleaseViews();

			/** Creates (or returns cached) descriptor of the requested type/surface. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetOrCreateView(const TextureSurface& surface, ViewType type);

			/** Creates an uncached descriptor of the requested type/surface. */
			D3D12_CPU_DESCRIPTOR_HANDLE CreateView(const TextureSurface& surface, ViewType type);

			/** Returns whether this texture uses an eagerly created complete-surface SRV. */
			bool UsesDefaultSRV() const;

			GpuDevice& mGpuDevice;
			D3D12Image* mImage = nullptr;
			DXGI_FORMAT mDXGIFormat = DXGI_FORMAT_UNKNOWN;

			D3D12_CPU_DESCRIPTOR_HANDLE mDefaultSRV{};
			TInlineArray<ViewEntry, 1> mViews;
			Mutex mViewMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
