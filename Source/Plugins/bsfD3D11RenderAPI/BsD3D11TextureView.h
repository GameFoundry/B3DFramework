//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsTextureView.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	DirectX implementation of a texture resource view. */
		class D3D11TextureView : public TextureView
		{
		public:
			~D3D11TextureView();

			/**
			 * Returns a shader resource view. Caller must take care this texture view actually contains a shader resource view,
			 * otherwise it returns null.
			 */
			ID3D11ShaderResourceView* GetSrv() const { return mSRV; }

			/**
			 * Returns a render target view. Caller must take care this texture view actually contains a render target view,
			 * otherwise it returns null.
			 */
			ID3D11RenderTargetView* GetRtv() const { return mRTV; }

			/**
			 * Returns a unordered access view. Caller must take care this texture view actually contains a unordered access
			 * view, otherwise it returns null.
			 */
			ID3D11UnorderedAccessView* GetUav() const { return mUAV; }

			/**
			 * Returns a depth stencil view. Caller must take care this texture view actually contains a depth stencil view,
			 * otherwise it returns null.
			 *
			 * @param[in]	readOnlyDepth	Should the view only support read operations for the depth portion of the
			 *								depth/stencil buffer(allows the bound texture to be also used as a shader resource
			 *								view while bound as a depth stencil target).
			 * @param[in]	readOnlyStencil	Should the view only support read operations for the stencil portion of the
			 *								depth/stencil buffer(allows the bound texture to be also used as a shader resource
			 *								view while bound as a depth stencil target).
			 */
			ID3D11DepthStencilView* GetDsv(bool readOnlyDepth, bool readOnlyStencil) const;

		protected:
			friend class D3D11Texture;

			D3D11TextureView(const D3D11Texture* texture, const TEXTURE_VIEW_DESC& desc);

		private:
			/**
			 * Creates a shader resource view that allows the provided surfaces to be bound as normal shader resources.
			 *
			 * @param[in]	texture			Texture to create the resource view for.
			 * @param[in]	mostDetailMip	First mip level to create the resource view for (0 - base level).
			 * @param[in]	numMips			Number of mip levels to create the view for.
			 * @param[in]	firstArraySlice	First array slice to create the view for. This will be array index for 1D and 2D
			 *								array textures, texture slice index for 3D textures, and face index for cube
			 *								textures (cube index * 6).
			 * @param[in]	numArraySlices	Number of array slices to create the view for. This will be number of array elements
			 *								for 1D and 2D array textures, number of slices for 3D textures, and number of cubes
			 *								for cube textures.
			 */
			ID3D11ShaderResourceView* CreateSrv(const D3D11Texture* texture, u32 mostDetailMip, u32 numMips, u32 firstArraySlice, u32 numArraySlices);

			/**
			 * Creates a shader resource view that allows the provided surfaces to be bound as render targets.
			 *
			 * @param[in]	texture			Texture to create the resource view for.
			 * @param[in]	mipSlice		Mip level to create the resource view for (0 - base level).
			 * @param[in]	firstArraySlice	First array slice to create the view for. This will be array index for 1D and 2D
			 *								array textures, texture slice index for 3D textures, and face index for cube
			 *								textures (cube index * 6).
			 * @param[in]	numArraySlices	Number of array slices to create the view for. This will be number of array elements
			 *								for 1D and 2D array textures, number of slices for 3D textures, and number of cubes
			 *								for cube textures.
			 */
			ID3D11RenderTargetView* CreateRtv(const D3D11Texture* texture, u32 mipSlice, u32 firstArraySlice, u32 numArraySlices);

			/**
			 * Creates a shader resource view that allows the provided surfaces to be bound as unordered access buffers.
			 *
			 * @param[in]	texture			Texture to create the resource view for.
			 * @param[in]	mipSlice		Mip level to create the resource view for (0 - base level).
			 * @param[in]	firstArraySlice	First array slice to create the view for. This will be array index for 1D and 2D
			 *								array textures, texture slice index for 3D textures, and face index for cube
			 *								textures (cube index * 6).
			 * @param[in]	numArraySlices	Number of array slices to create the view for. This will be number of array elements
			 *								for 1D and 2D array textures, number of slices for 3D textures, and number of cubes
			 *								for cube textures.
			 */
			ID3D11UnorderedAccessView* CreateUav(const D3D11Texture* texture, u32 mipSlice, u32 firstArraySlice, u32 numArraySlices);

			/**
			 * Creates a shader resource view that allows the provided surfaces to be bound as depth stencil buffers.
			 *
			 * @param[in]	texture			Texture to create the resource view for.
			 * @param[in]	mipSlice		Mip level to create the resource view for (0 - base level).
			 * @param[in]	firstArraySlice	First array slice to create the view for. This will be array index for 1D and 2D
			 *								array textures, texture slice index for 3D textures, and face index for cube
			 *								textures (cube index * 6).
			 * @param[in]	numArraySlices	Number of array slices to create the view for. This will be number of array elements
			 *								for 1D and 2D array textures, number of slices for 3D textures, and number of cubes
			 *								for cube textures.
			 * @param[in]	readOnlyDepth	Should the view only support read operations for the depth portion of the
			 *								depth/stencil buffer(allows the bound texture to be also used as a shader resource
			 *								view while bound as a depth stencil target).
			 * @param[in]	readOnlyStencil	Should the view only support read operations for the stencil portion of the
			 *								depth/stencil buffer(allows the bound texture to be also used as a shader resource
			 *								view while bound as a depth stencil target).
			 */
			ID3D11DepthStencilView* CreateDsv(const D3D11Texture* texture, u32 mipSlice, u32 firstArraySlice, u32 numArraySlices, bool readOnlyDepth, bool readOnlyStencil);

			ID3D11ShaderResourceView* mSRV = nullptr;
			ID3D11RenderTargetView* mRTV = nullptr;
			ID3D11UnorderedAccessView* mUAV = nullptr;
			ID3D11DepthStencilView* mWDepthWStencilView = nullptr;
			ID3D11DepthStencilView* mRODepthWStencilView = nullptr;
			ID3D11DepthStencilView* mRODepthROStencilView = nullptr;
			ID3D11DepthStencilView* mWDepthROStencilView = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
