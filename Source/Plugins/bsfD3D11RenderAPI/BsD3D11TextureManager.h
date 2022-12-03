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
		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma) override;

	protected:
		SPtr<RenderTexture> CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) override;
	};

	namespace ct
	{
		/**	Handles creation of DirectX 11 textures. */
		class D3D11TextureManager : public TextureManager
		{
		protected:
			SPtr<Texture> CreateTextureInternal(const TextureCreateInformation& desc, const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx = 0) override;
		};
	} // namespace ct

	/** @} */
} // namespace bs
