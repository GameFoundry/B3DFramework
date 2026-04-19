//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "Image/B3DTexture.h"
#include "B3DD3D12Resource.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
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

			/** Returns the D3D12 resource. */
			ID3D12Resource* GetD3D12Resource() const override { return mTexture.Get(); }

			/** Returns the DXGI format of the texture. */
			DXGI_FORMAT GetDXGIFormat() const { return mDXGIFormat; }

		protected:
			/** @copydoc Texture::Map */
			PixelData Map(GpuResourceUsage usage, u32 face, u32 mipLevel) override;

			/** @copydoc Texture::Unmap */
			void Unmap(u32 face, u32 mipLevel) override;

			/** @copydoc Texture::WriteData */
			void WriteData(const PixelData& data, u32 face, u32 mipLevel, bool discardEntireBuffer) override;

			/** @copydoc Texture::ReadData */
			void ReadData(PixelData& data, u32 face, u32 mipLevel) override;

			/** @copydoc Texture::CopyData */
			void CopyData(Texture& destination, const PixelData& srcData, const PixelData& dstData) override;

		private:
			/** Creates the D3D12 texture resource. */
			void CreateTexture();

			/** Creates a staging buffer for CPU access to the specified subresource. */
			void CreateStagingBuffer(u32 subresourceIndex, u32 width, u32 height);

			/** Calculates the subresource index for the given face and mip level. */
			u32 CalculateSubresourceIndex(u32 face, u32 mipLevel) const;

			ComPtr<ID3D12Resource> mTexture;
			D3D12MA::Allocation* mAllocation = nullptr;
			DXGI_FORMAT mDXGIFormat;

			// Staging resources for CPU access
			ComPtr<ID3D12Resource> mStagingBuffer;
			void* mMappedData = nullptr;
			u32 mMappedSubresource = (u32)-1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
