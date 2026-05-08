//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "Managers/B3DTextureManager.h"

namespace b3d
{
	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/** Metal implementation of TextureManager. */
	class MetalTextureManager : public TextureManager
	{
	public:
		MetalTextureManager() = default;
		~MetalTextureManager() = default;

		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma) override;

	protected:
		TShared<RenderTexture> CreateRenderTextureImpl(const RenderTextureCreateInformation& createInformation) override;
	};

	namespace render
	{
		/** Metal implementation of TextureManager for the render thread. */
		class MetalTextureManager : public TextureManager
		{
		public:
			MetalTextureManager(GpuDevice& gpuDevice);
			~MetalTextureManager() = default;

		protected:
			TShared<RenderTexture> CreateRenderTextureInternal(const RenderTextureCreateInformation& createInformation) override;
		};
	} // namespace render

	/** @} */
} // namespace b3d
