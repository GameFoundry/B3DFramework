//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "Managers/B3DTextureManager.h"

namespace b3d
{
	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/**	Handles creation of DirectX 12 textures on the simulation thread. */
	class D3D12TextureManager : public TextureManager
	{
	public:
		/** @copydoc TextureManager::GetNativeFormat */
		PixelFormat GetNativeFormat(TextureType textureType, PixelFormat format, TextureUsageFlags usage, bool hardwareGamma) override;

	protected:
		/** @copydoc TextureManager::CreateRenderTextureImpl */
		TShared<RenderTexture> CreateRenderTextureImpl(const RenderTextureCreateInformation& createInformation) override;
	};

	namespace render
	{
		/**	Handles creation of DirectX 12 textures on the core thread. */
		class D3D12TextureManager : public TextureManager
		{
		public:
			D3D12TextureManager(GpuDevice& gpuDevice)
				: TextureManager(gpuDevice)
			{ }

		protected:
			/** @copydoc TextureManager::CreateRenderTextureInternal */
			TShared<RenderTexture> CreateRenderTextureInternal(const RenderTextureCreateInformation& createInformation) override;
		};
	} // namespace render

	/** @} */
} // namespace b3d
