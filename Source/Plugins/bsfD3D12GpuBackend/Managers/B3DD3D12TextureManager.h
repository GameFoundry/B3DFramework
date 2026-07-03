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

	/**	Handles creation of DirectX 12 textures. */
	class D3D12TextureManager : public TextureManager
	{
	public:
		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma) override;

	protected:
		TShared<RenderTexture> CreateRenderTextureImpl(const RenderTextureCreateInformation& desc) override;
	};

	namespace render
	{
		/**	Handles creation of DirectX 12 textures. */
		class D3D12TextureManager : public TextureManager
		{
		public:
			D3D12TextureManager(GpuDevice& gpuDevice)
				:TextureManager(gpuDevice)
			{ }

		protected:
			TShared<RenderTexture> CreateRenderTextureInternal(const RenderTextureCreateInformation& desc) override;
		};
	} // namespace render

	/** @} */
} // namespace b3d
