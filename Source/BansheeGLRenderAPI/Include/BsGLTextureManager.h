//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "BsTextureManager.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles creation of OpenGL textures. */
    class BS_RSGL_EXPORT GLTextureManager : public TextureManager
    {
    public:
        GLTextureManager(ct::GLSupport& support);
        virtual ~GLTextureManager();

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
	class BS_RSGL_EXPORT GLTextureManager : public TextureManager
	{
	public:
		GLTextureManager(GLSupport& support);

	protected:		
		/** @copydoc TextureManager::createTextureInternal */
		SPtr<TextureCore> createTextureInternal(const TEXTURE_DESC& desc,
			const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc TextureManager::createRenderTextureInternal */
		SPtr<RenderTextureCore> createRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, 
			UINT32 deviceIdx = 0) override;

		GLSupport& mGLSupport;
	};
	}

	/** @} */
}