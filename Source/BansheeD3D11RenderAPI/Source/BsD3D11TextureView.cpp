//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11TextureView.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11Texture.h"
#include "BsRenderStats.h"
#include "BsD3D11Mappings.h"
#include "BsException.h"

namespace BansheeEngine
{
	D3D11TextureView::D3D11TextureView(const SPtr<TextureCore>& texture, const TEXTURE_VIEW_DESC& desc)
		:TextureView(texture, desc), mSRV(nullptr), mUAV(nullptr), mDSV(nullptr), mRTV(nullptr), mRODSV(nullptr)
	{
		D3D11TextureCore* d3d11Texture = static_cast<D3D11TextureCore*>(mOwnerTexture.get());

		if ((mDesc.usage & GVU_RANDOMWRITE) != 0)
			mUAV = createUAV(d3d11Texture, mDesc.mostDetailMip, mDesc.firstArraySlice, mDesc.numArraySlices);
		else if ((mDesc.usage & GVU_RENDERTARGET) != 0)
			mRTV = createRTV(d3d11Texture, mDesc.mostDetailMip, mDesc.firstArraySlice, mDesc.numArraySlices);
		else if ((mDesc.usage & GVU_DEPTHSTENCIL) != 0)
		{
			mDSV = createDSV(d3d11Texture, mDesc.mostDetailMip, mDesc.firstArraySlice, mDesc.numArraySlices, false);
			mRODSV = createDSV(d3d11Texture, mDesc.mostDetailMip, mDesc.firstArraySlice, mDesc.numArraySlices, true);
		}
		else
			mSRV = createSRV(d3d11Texture, mDesc.mostDetailMip, mDesc.numMips, mDesc.firstArraySlice, mDesc.numArraySlices);
	}

	D3D11TextureView::~D3D11TextureView()
	{
		SAFE_RELEASE(mSRV);
		SAFE_RELEASE(mUAV);
		SAFE_RELEASE(mDSV);
		SAFE_RELEASE(mRODSV);
		SAFE_RELEASE(mRTV);
	}

	ID3D11ShaderResourceView* D3D11TextureView::createSRV(D3D11TextureCore* texture, 
		UINT32 mostDetailMip, UINT32 numMips, UINT32 firstArraySlice, UINT32 numArraySlices)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		const TextureProperties& texProps = texture->getProperties();
		UINT32 numFaces = texProps.getNumFaces();

		switch (texProps.getTextureType())
		{
		case TEX_TYPE_1D:
			if (numFaces <= 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipLevels = numMips;
				desc.Texture1D.MostDetailedMip = mostDetailMip;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipLevels = numMips;
				desc.Texture1DArray.MostDetailedMip = mostDetailMip;
				desc.Texture1DArray.FirstArraySlice = firstArraySlice;
				desc.Texture1DArray.ArraySize = numArraySlices;
			}
			break;
		case TEX_TYPE_2D:
			if (texProps.getNumSamples() > 1)
			{
				if (numFaces <= 1)
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
					desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
					desc.Texture2DMSArray.ArraySize = numArraySlices;
				}
			}
			else
			{
				if (numFaces <= 1)
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipLevels = numMips;
					desc.Texture2D.MostDetailedMip = mostDetailMip;
				}
				else
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.MipLevels = numMips;
					desc.Texture2DArray.MostDetailedMip = mostDetailMip;
					desc.Texture2DArray.FirstArraySlice = firstArraySlice;
					desc.Texture2DArray.ArraySize = numArraySlices;
				}
			}
			break;
		case TEX_TYPE_3D:
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipLevels = numMips;
			desc.Texture3D.MostDetailedMip = mostDetailMip;
			break;
		case TEX_TYPE_CUBE_MAP:
			if (numFaces <= 6)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				desc.TextureCube.MipLevels = numMips;
				desc.TextureCube.MostDetailedMip = mostDetailMip;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
				desc.TextureCubeArray.MipLevels = numMips;
				desc.TextureCubeArray.MostDetailedMip = mostDetailMip;
				desc.TextureCubeArray.First2DArrayFace = firstArraySlice;
				desc.TextureCubeArray.NumCubes = numArraySlices / 6;
			}
			break;
		default:
			BS_EXCEPT(InvalidParametersException, "Invalid texture type for this view type.");
		}

		desc.Format = texture->getColorFormat();

		ID3D11ShaderResourceView* srv = nullptr;

		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());
		HRESULT hr = d3d11rs->getPrimaryDevice().getD3D11Device()->CreateShaderResourceView(texture->getDX11Resource(), &desc, &srv);

		if (FAILED(hr) || d3d11rs->getPrimaryDevice().hasError())
		{
			String msg = d3d11rs->getPrimaryDevice().getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create ShaderResourceView: " + msg);
		}

		return srv;
	}

	ID3D11RenderTargetView* D3D11TextureView::createRTV(D3D11TextureCore* texture,
		UINT32 mipSlice, UINT32 firstArraySlice, UINT32 numArraySlices)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		const TextureProperties& texProps = texture->getProperties();
		UINT32 numFaces = texProps.getNumFaces();

		switch (texProps.getTextureType())
		{
		case TEX_TYPE_1D:
			if (numFaces <= 1)
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipSlice = mipSlice;
			}
			else
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipSlice = mipSlice;
				desc.Texture1DArray.FirstArraySlice = firstArraySlice;
				desc.Texture1DArray.ArraySize = numArraySlices;
			}
			break;
		case TEX_TYPE_2D:
			if (texProps.getNumSamples() > 1)
			{
				if (numFaces <= 1)
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
					desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
					desc.Texture2DMSArray.ArraySize = numArraySlices;
				}
			}
			else
			{
				if (numFaces <= 1)
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipSlice = mipSlice;
				}
				else
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.MipSlice = mipSlice;
					desc.Texture2DArray.FirstArraySlice = firstArraySlice;
					desc.Texture2DArray.ArraySize = numArraySlices;
				}
			}
			break;
		case TEX_TYPE_3D:
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipSlice = mipSlice;
			desc.Texture3D.FirstWSlice = firstArraySlice;
			desc.Texture3D.WSize = numArraySlices;
			break;
		case TEX_TYPE_CUBE_MAP:
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = firstArraySlice;
			desc.Texture2DArray.ArraySize = numArraySlices;
			desc.Texture2DArray.MipSlice = mipSlice;
			break;
		default:
			BS_EXCEPT(InvalidParametersException, "Invalid texture type for this view type.");
		}

		desc.Format = texture->getColorFormat();

		ID3D11RenderTargetView* rtv = nullptr;

		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());
		HRESULT hr = d3d11rs->getPrimaryDevice().getD3D11Device()->CreateRenderTargetView(texture->getDX11Resource(), &desc, &rtv);

		if (FAILED(hr) || d3d11rs->getPrimaryDevice().hasError())
		{
			String msg = d3d11rs->getPrimaryDevice().getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create RenderTargetView: " + msg);
		}

		return rtv;
	}

	ID3D11UnorderedAccessView* D3D11TextureView::createUAV(D3D11TextureCore* texture,
		UINT32 mipSlice, UINT32 firstArraySlice, UINT32 numArraySlices)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		const TextureProperties& texProps = texture->getProperties();
		UINT32 numFaces = texProps.getNumFaces();

		switch (texProps.getTextureType())
		{
		case TEX_TYPE_1D:
			if (numFaces <= 1)
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipSlice = mipSlice;
			}
			else
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipSlice = mipSlice;
				desc.Texture1DArray.FirstArraySlice = firstArraySlice;
				desc.Texture1DArray.ArraySize = numArraySlices;
			}
			break;
		case TEX_TYPE_2D:
			if (numFaces <= 1)
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = mipSlice;
			}
			else
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = mipSlice;
				desc.Texture2DArray.FirstArraySlice = firstArraySlice;
				desc.Texture2DArray.ArraySize = numArraySlices;
			}
			break;
		case TEX_TYPE_3D:
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipSlice = mipSlice;
			desc.Texture3D.FirstWSlice = firstArraySlice;
			desc.Texture3D.WSize = numArraySlices;
			break;
		case TEX_TYPE_CUBE_MAP:
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = firstArraySlice;
			desc.Texture2DArray.ArraySize = numArraySlices;
			desc.Texture2DArray.MipSlice = mipSlice;
			break;
		default:
			BS_EXCEPT(InvalidParametersException, "Invalid texture type for this view type.");
		}

		desc.Format = texture->getDXGIFormat();

		ID3D11UnorderedAccessView* uav = nullptr;

		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());
		HRESULT hr = d3d11rs->getPrimaryDevice().getD3D11Device()->CreateUnorderedAccessView(texture->getDX11Resource(), &desc, &uav);

		if (FAILED(hr) || d3d11rs->getPrimaryDevice().hasError())
		{
			String msg = d3d11rs->getPrimaryDevice().getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create UnorderedAccessView: " + msg);
		}

		return uav;
	}

	ID3D11DepthStencilView* D3D11TextureView::createDSV(D3D11TextureCore* texture,
		UINT32 mipSlice, UINT32 firstArraySlice, UINT32 numArraySlices, bool readOnly)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		const TextureProperties& texProps = texture->getProperties();
		UINT32 numFaces = texProps.getNumFaces();

		switch (texProps.getTextureType())
		{
		case TEX_TYPE_1D:
			if (numFaces <= 1)
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipSlice = mipSlice;
			}
			else
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipSlice = mipSlice;
				desc.Texture1DArray.FirstArraySlice = firstArraySlice;
				desc.Texture1DArray.ArraySize = numArraySlices;
			}
			break;
		case TEX_TYPE_2D:
			if (texProps.getNumSamples() > 1)
			{
				if (numFaces <= 1)
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
					desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
					desc.Texture2DMSArray.ArraySize = numArraySlices;
				}
			}
			else
			{
				if (numFaces <= 1)
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipSlice = mipSlice;
				}
				else
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.MipSlice = mipSlice;
					desc.Texture2DArray.FirstArraySlice = firstArraySlice;
					desc.Texture2DArray.ArraySize = numArraySlices;
				}
			}
			break;
		case TEX_TYPE_3D:
		case TEX_TYPE_CUBE_MAP:
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = firstArraySlice;
			desc.Texture2DArray.ArraySize = numArraySlices;
			desc.Texture2DArray.MipSlice = mipSlice;
			break;
		default:
			BS_EXCEPT(InvalidParametersException, "Invalid texture type for this view type.");
		}

		if (readOnly)
			desc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;

		desc.Format = texture->getDepthStencilFormat();

		ID3D11DepthStencilView* dsv = nullptr;

		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(D3D11RenderAPI::instancePtr());
		HRESULT hr = d3d11rs->getPrimaryDevice().getD3D11Device()->CreateDepthStencilView(texture->getDX11Resource(), &desc, &dsv);

		if (FAILED(hr) || d3d11rs->getPrimaryDevice().hasError())
		{
			String msg = d3d11rs->getPrimaryDevice().getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create DepthStencilView: " + msg);
		}

		return dsv;
	}
}