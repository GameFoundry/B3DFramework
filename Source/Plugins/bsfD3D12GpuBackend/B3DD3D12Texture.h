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

		/** DirectX 12 implementation of a texture. */
		class D3D12Texture : public Texture, public D3D12Resource
		{
		public:
			D3D12Texture(const TextureCreateInformation& createInformation, GpuDevice& device);
			~D3D12Texture() override;

			/** @copydoc Texture::Initialize */
			void Initialize() override;

			/** @copydoc D3D12Resource::GetD3D12Resource */
			ID3D12Resource* GetD3D12Resource() const override { return mTexture.Get(); }

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

			/** Creates the D3D12 texture resource. */
			void CreateTexture();

			/** Releases the currently allocated D3D12 texture resource, if any. */
			void ReleaseTexture();

			/** Frees all cached view descriptors. */
			void ReleaseViews();

			/** Creates (or returns cached) descriptor of the requested type/surface. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetOrCreateView(const TextureSurface& surface, ViewType type);

			GpuDevice& mGpuDevice;
			ComPtr<ID3D12Resource> mTexture;
			D3D12MA::Allocation* mAllocation = nullptr;
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
