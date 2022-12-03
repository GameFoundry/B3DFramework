//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "Managers/BsTextureManager.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles creation of OpenGL textures. */
	class GLTextureManager : public TextureManager
	{
	public:
		GLTextureManager(ct::GLSupport& support);
		virtual ~GLTextureManager() = default;

		/**
		 * Converts the provided format for the specified texture type and usage into a format that is supported by OpenGL.
		 */
		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma);

	protected:
		SPtr<RenderTexture> CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) override;

		ct::GLSupport& mGLSupport;
	};

	namespace ct
	{
		/** Handles creation of OpenGL textures. */
		class GLTextureManager : public TextureManager
		{
		public:
			GLTextureManager(GLSupport& support);

		protected:
			SPtr<Texture> CreateTextureInternal(const TextureCreateInformation& desc, const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx = 0) override;

			GLSupport& mGLSupport;
		};
	} // namespace ct

	/** @} */
} // namespace bs
