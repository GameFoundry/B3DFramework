//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Managers/BsTextureManager.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Handles creation of DirectX 11 textures. */
	class D3D11TextureManager : public TextureManager
	{
	public:
		/** @copydoc TextureManager::getNativeFormat */
		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma);

	protected:
		/** @copydoc TextureManager::createRenderTextureImpl */
		SPtr<RenderTexture> CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc);
	};

	namespace ct
	{
		/**	Handles creation of DirectX 11 textures. */
		class D3D11TextureManager : public TextureManager
		{
		protected:
			/** @copydoc TextureManager::createTextureInternal */
			SPtr<Texture> CreateTextureInternal(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc TextureManager::createRenderTextureInternal */
			SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx = 0);
		};
	} // namespace ct

	/** @} */
} // namespace bs
