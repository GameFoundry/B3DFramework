//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "Managers/B3DTextureManager.h"

namespace b3d
{
	/** @addtogroup NullGpuBackend 
	 *  @{
	 */

	/** Null implementation of TextureManager. */
	class NullTextureManager : public TextureManager
	{
	public:
		NullTextureManager() = default;
		~NullTextureManager() = default;

		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma) override;

	protected:
		SPtr<RenderTexture> CreateRenderTextureImpl(const RenderTextureCreateInformation& createInformation) override;
	};

	namespace render
	{
		/** Null implementation of TextureManager for the render thread. */
		class NullTextureManager : public TextureManager
		{
		public:
			NullTextureManager(GpuDevice& gpuDevice);
			~NullTextureManager() = default;

		protected:
			SPtr<RenderTexture> CreateRenderTextureInternal(const RenderTextureCreateInformation& createInformation) override;
		};
	} // namespace render

	/** @} */
} // namespace b3d
