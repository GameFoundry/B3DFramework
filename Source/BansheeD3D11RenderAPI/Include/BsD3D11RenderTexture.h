//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsTexture.h"
#include "BsRenderTexture.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**
	 * DirectX 11 implementation of a render texture.
	 *
	 * @note	Sim thread only.
	 */
	class D3D11RenderTexture : public RenderTexture
	{
	public:
		virtual ~D3D11RenderTexture() { }

	protected:
		friend class D3D11TextureManager;

		D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc);

		/** @copydoc RenderTexture::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};

	namespace ct
	{
	/**
	 * DirectX 11 implementation of a render texture.
	 *
	 * @note	Core thread only.
	 */
	class D3D11RenderTexture : public RenderTextureCore
	{
	public:
		D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc, UINT32 deviceIdx);
		virtual ~D3D11RenderTexture() { }

		/** @copydoc RenderTextureCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* data) const override;

	protected:
		/** @copydoc RenderTextureCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};
	}

	/** @} */
}