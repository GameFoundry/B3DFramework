//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsTextureView.h"

namespace bs { namespace ct
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
		ID3D11ShaderResourceView* getSRV() const { return mSRV; }

		/**
		 * Returns a render target view. Caller must take care this texture view actually contains a render target view,
		 * otherwise it returns null.
		 */
		ID3D11RenderTargetView*	getRTV() const { return mRTV; }

		/**
		 * Returns a unordered access view. Caller must take care this texture view actually contains a unordered access
		 * view, otherwise it returns null.
		 */
		ID3D11UnorderedAccessView* getUAV() const { return mUAV; }

		/**
		 * Returns a depth stencil view. Caller must take care this texture view actually contains a depth stencil view,
		 * otherwise it returns null.
		 *
		 * @param[in]	readOnly	Should the depth stencil view only support read operations (allows the bound texture to
		 *							be also used as a shader resource view while bound as a depth stencil target).
		 */
		ID3D11DepthStencilView*	getDSV(bool readOnly) const { return readOnly ? mRODSV : mDSV; }

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
		ID3D11ShaderResourceView* createSRV(const D3D11Texture* texture, 
			UINT32 mostDetailMip, UINT32 numMips, UINT32 firstArraySlice, UINT32 numArraySlices);

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
		ID3D11RenderTargetView* createRTV(const D3D11Texture* texture,
			UINT32 mipSlice, UINT32 firstArraySlice, UINT32 numArraySlices);

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
		ID3D11UnorderedAccessView* createUAV(const D3D11Texture* texture,
			UINT32 mipSlice, UINT32 firstArraySlice, UINT32 numArraySlices);

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
		 * @param[in]	readOnly		Should the depth stencil view only support read operations (allows the bound texture
		 *								to be also used as a shader resource view while bound as a depth stencil target).
		 */
		ID3D11DepthStencilView* createDSV(const D3D11Texture* texture,
			UINT32 mipSlice, UINT32 firstArraySlice, UINT32 numArraySlices, bool readOnly);

		ID3D11ShaderResourceView* mSRV;
		ID3D11RenderTargetView* mRTV;
		ID3D11UnorderedAccessView* mUAV;
		ID3D11DepthStencilView*	mDSV;
		ID3D11DepthStencilView*	mRODSV;
	};

	/** @} */
}}