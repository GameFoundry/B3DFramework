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
		PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma) override;

	protected:
		/** @copydoc TextureManager::createRenderTextureImpl */
		SPtr<RenderTexture> createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) override;

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
		/** @copydoc TextureManager::createTextureInternal */
		SPtr<Texture> createTextureInternal(const TEXTURE_DESC& desc,
			const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc TextureManager::createRenderTextureInternal */
		SPtr<RenderTexture> createRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, 
			UINT32 deviceIdx = 0) override;

		GLSupport& mGLSupport;
	};
	}

	/** @} */
}