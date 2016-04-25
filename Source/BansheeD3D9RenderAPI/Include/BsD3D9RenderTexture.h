//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsD3D9Resource.h"
#include "BsRenderTexture.h"

namespace BansheeEngine
{
	/** @addtogroup D3D9
	 *  @{
	 */

	class D3D9RenderTexture;

	/**
	 * DirectX 9 implementation of a render texture.
	 *
	 * @note	Core thread only.
	 */
	class BS_D3D9_EXPORT D3D9RenderTextureCore : public RenderTextureCore, public D3D9Resource
	{
	public:
		D3D9RenderTextureCore(const RENDER_TEXTURE_CORE_DESC& desc);

		virtual ~D3D9RenderTextureCore();

		/** @copydoc RenderTextureCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* data) const override;

		/** @copydoc D3D9Resource::notifyOnDeviceCreate */
		void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device) override;

		/** @copydoc D3D9Resource::notifyOnDeviceDestroy */
		void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device) override;

		/** @copydoc D3D9Resource::notifyOnDeviceLost */
		void notifyOnDeviceLost(IDirect3DDevice9* d3d9Device) override;

		/** @copydoc D3D9Resource::notifyOnDeviceReset */
		void notifyOnDeviceReset(IDirect3DDevice9* d3d9Device) override;

	protected:
		friend class D3D9RenderTexture;

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**	Initializes the internal color and depth surfaces. */
		void initializeSurfaces();

		/**	Releases the internal color and depth surfaces. */
		void releaseSurfaces();

		/** @copydoc RenderTextureCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		IDirect3DSurface9* mDX9ColorSurface;
		IDirect3DSurface9* mDX9DepthStencilSurface;
		bool mIsBindableToShader;
		RenderTextureProperties mProperties;
	};

	/**
	 * DirectX 9 implementation of a render texture.
	 *
	 * @note	Sim thread only.
	 */
	class D3D9RenderTexture : public RenderTexture
	{
	public:
		virtual ~D3D9RenderTexture() { }

	protected:
		friend class D3D9TextureManager;

		D3D9RenderTexture(const RENDER_TEXTURE_DESC& desc);

		/** @copydoc RenderTexture::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};

	/** @} */
}