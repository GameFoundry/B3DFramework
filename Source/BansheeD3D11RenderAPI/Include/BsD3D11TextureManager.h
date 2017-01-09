//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsTextureManager.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Handles creation of DirectX 11 textures. */
	class BS_D3D11_EXPORT D3D11TextureManager : public TextureManager
	{
	public:
		/** @copydoc TextureManager::getNativeFormat */
		PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma) override;

	protected:		
		/** @copydoc TextureManager::createRenderTextureImpl */
		SPtr<RenderTexture> createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) override;
	};

	namespace ct
	{
	/**	Handles creation of DirectX 11 textures. */
	class BS_D3D11_EXPORT D3D11TextureManager : public TextureCoreManager
	{
	protected:
		/** @copydoc TextureCoreManager::createTextureInternal */
		SPtr<TextureCore> createTextureInternal(const TEXTURE_DESC& desc, 
			const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc TextureCoreManager::createRenderTextureInternal */
		SPtr<RenderTextureCore> createRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, 
			UINT32 deviceIdx = 0) override;
	};
	}

	/** @} */
}