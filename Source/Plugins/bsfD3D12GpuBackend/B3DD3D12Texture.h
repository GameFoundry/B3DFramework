//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "Image/B3DTexture.h"
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
		 * individually by the resource tracker. Also stores the subresource's current native resource state, which
		 * the barrier helper reads and advances when it emits transitions.
		 */
		class D3D12ImageSubresource : public D3D12Resource
		{
		public:
			D3D12ImageSubresource(D3D12ResourceManager* owner, D3D12_RESOURCE_STATES state, const StringView& name = "");

			/**
			 * Returns the current native state of the subresource.
			 *
			 * @note Assumes single-threaded command recording per resource (render thread + internal work context);
			 *       there is no cross-command-buffer synchronization on this field.
			 */
			D3D12_RESOURCE_STATES GetState() const { return mState; }

			/** Sets the current native state of the subresource. */
			void SetState(D3D12_RESOURCE_STATES state) { mState = state; }

		private:
			D3D12_RESOURCE_STATES mState;
		};

		/** Descriptor structure used for initialization of a D3D12Image. */
		struct D3D12ImageCreateInformation
		{
			/** Native resource wrapped by the image. */
			ComPtr<ID3D12Resource> Resource;

			/** Memory allocation backing the resource, or null for externally owned resources (swap-chain buffers). */
			D3D12MA::Allocation* Allocation = nullptr;

			/** Format of the resource. */
			DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;

			/** Resource state the native resource was created in (all subresources). */
			D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON;

			/** Number of array slices (or cube faces) in the image. */
			u32 FaceCount = 1;

			/** Number of mip levels in the image. */
			u32 MipLevelCount = 1;

			/** Which aspects (color/depth/stencil) the image format contains. */
			GpuTextureAspectFlags Aspect = GpuTextureAspectFlag::Color;

			/** Optional debug name. */
			String Name;
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
			D3D12MA::Allocation* mAllocation = nullptr;
			DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
		};

		/** DirectX 12 implementation of a texture. */
		class D3D12Texture : public Texture
		{
		public:
			D3D12Texture(const TextureCreateInformation& createInformation, GpuDevice& device);
			~D3D12Texture() override;

			/** @copydoc Texture::Initialize */
			void Initialize() override;

			/** Returns the low-level image resource wrapping the native D3D12 texture. */
			D3D12Image* GetD3D12Image() const { return mImage; }

			/** Returns the native D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const { return mImage != nullptr ? mImage->GetD3D12Resource() : nullptr; }

			/** Returns the DXGI format of the texture. */
			DXGI_FORMAT GetDXGIFormat() const { return mDXGIFormat; }

			/** @copydoc render::Texture::GetDevice */
			GpuDevice& GetDevice() const override { return mGpuDevice; }

			/** @copydoc render::Texture::Map */
			GpuTextureMappedScope Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options) override;

			/** @copydoc render::Texture::RecreateInternalTexture */
			void RecreateInternalTexture() override;

			/** @copydoc render::Texture::GetStagingBufferPitchForSubresource */
			ImageSubresourcePitch GetStagingBufferPitchForSubresource(u32 face, u32 mipLevel) const override;

			/**
			 * Byte pitch between rows that staging buffers use for copies to or from the given subresource. Padded to
			 * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT as required for placed copy footprints, while remaining a whole
			 * number of format blocks.
			 */
			u32 GetStagingRowPitchInBytes(u32 mipLevel) const;

			/** @copydoc render::Texture::GetUseMask */
			GpuQueueMask GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags = GpuAccessFlag::Read | GpuAccessFlag::Write) const override;

			/** @copydoc render::Texture::GetBoundCount */
			u32 GetBoundCount(u32 subresourceIdx = 0) const override;

			/** @copydoc render::Texture::GetUseCount */
			u32 GetUseCount(u32 subresourceIdx = 0) const override;

			/** @copydoc render::Texture::Flush */
			void Flush(u32 mipLevel, u32 arrayLayer) override;

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
				SRV,
				UAV
			};

			/** Key used to cache texture views by surface and view type. */
			struct ViewKey
			{
				TextureSurface Surface;
				ViewType Type;

				bool operator==(const ViewKey& other) const
				{
					return Type == other.Type && Surface == other.Surface;
				}
			};

			/** Hashes a ViewKey. */
			struct ViewKeyHash
			{
				size_t operator()(const ViewKey& key) const;
			};

			/** Creates the D3D12 texture resource and its D3D12Image wrapper. */
			void CreateTexture();

			/** Queues the current D3D12Image (if any) for deferred destruction and drops all cached views. */
			void ReleaseTexture();

			/** Frees all cached view descriptors. */
			void ReleaseViews();

			/** Creates (or returns cached) descriptor of the requested type/surface. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetOrCreateView(const TextureSurface& surface, ViewType type);

			GpuDevice& mGpuDevice;
			D3D12Image* mImage = nullptr;
			DXGI_FORMAT mDXGIFormat = DXGI_FORMAT_UNKNOWN;

			UnorderedMap<ViewKey, D3D12_CPU_DESCRIPTOR_HANDLE, ViewKeyHash> mViews;

			// CPU-side buffer backing the most recent write mapping. Uploaded to the GPU on Flush(). See Map().
			TShared<PixelData> mMappedWriteData;
			u32 mMappedWriteMip = 0;
			u32 mMappedWriteLayer = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
