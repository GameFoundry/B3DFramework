//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D9RenderTexture.h"
#include "BsD3D9Texture.h"
#include "BsD3D9PixelBuffer.h"
#include "BsD3D9RenderAPI.h"
#include "BsTextureView.h"

namespace BansheeEngine
{
	D3D9RenderTextureCore::D3D9RenderTextureCore(const RENDER_TEXTURE_CORE_DESC& desc)
		:RenderTextureCore(desc), mProperties(desc, false), mDX9ColorSurface(nullptr),
		mDX9DepthStencilSurface(nullptr), mIsBindableToShader(false)
	{ }

	D3D9RenderTextureCore::~D3D9RenderTextureCore()
	{ }

	void D3D9RenderTextureCore::initialize()
	{
		RenderTextureCore::initialize();

		initializeSurfaces();
	}

	void D3D9RenderTextureCore::initializeSurfaces()
	{
		D3D9TextureCore* d3d9texture = static_cast<D3D9TextureCore*>(mColorSurface->getTexture().get());
		D3D9PixelBuffer* pixelBuffer = static_cast<D3D9PixelBuffer*>(
			d3d9texture->getBuffer(mColorSurface->getFirstArraySlice(), mColorSurface->getMostDetailedMip()).get());
		mDX9ColorSurface = pixelBuffer->getSurface(D3D9RenderAPI::getActiveD3D9Device());

		D3D9TextureCore* d3d9DepthStencil = static_cast<D3D9TextureCore*>(mDepthStencilSurface->getTexture().get());
		D3D9PixelBuffer* depthStencilBuffer = static_cast<D3D9PixelBuffer*>(
			d3d9DepthStencil->getBuffer(mDepthStencilSurface->getFirstArraySlice(), mDepthStencilSurface->getMostDetailedMip()).get());
		mDX9DepthStencilSurface = depthStencilBuffer->getSurface(D3D9RenderAPI::getActiveD3D9Device());
	}

	void D3D9RenderTextureCore::releaseSurfaces()
	{
		// All actual releasing happens in the color and depth textures.
		mDX9ColorSurface = nullptr;
		mDX9DepthStencilSurface = nullptr;
	}

	void D3D9RenderTextureCore::getCustomAttribute(const String& name, void* data) const
	{
		if(name == "DDBACKBUFFER")
		{
			IDirect3DSurface9** surf = (IDirect3DSurface9**)data;
			*surf = mDX9ColorSurface;
		}
		else if(name == "D3DZBUFFER")
		{
			IDirect3DSurface9** surf = (IDirect3DSurface9**)data;
			*surf = mDX9DepthStencilSurface;
		}
		else if(name == "HWND")
		{
			HWND* hWnd = (HWND*)data;
			*hWnd = nullptr;
		}
	}

	void D3D9RenderTextureCore::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{
		initializeSurfaces();
	}

	void D3D9RenderTextureCore::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{
		releaseSurfaces();
	}

	void D3D9RenderTextureCore::notifyOnDeviceLost(IDirect3DDevice9* d3d9Device)
	{
		releaseSurfaces();
	}

	void D3D9RenderTextureCore::notifyOnDeviceReset(IDirect3DDevice9* d3d9Device)
	{
		initializeSurfaces();
	}

	D3D9RenderTexture::D3D9RenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, false)
	{

	}
}