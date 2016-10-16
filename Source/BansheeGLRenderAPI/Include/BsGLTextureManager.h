//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "BsTextureManager.h"

namespace BansheeEngine 
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles creation of OpenGL textures. */
    class BS_RSGL_EXPORT GLTextureManager : public TextureManager
    {
    public:
        GLTextureManager(GLSupport& support);
        virtual ~GLTextureManager();

		/**
		 * Converts the provided format for the specified texture type and usage into a format that is supported by OpenGL.
		 */
		PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma) override;

	protected:
		/** @copydoc TextureManager::createRenderTextureImpl */
		SPtr<RenderTexture> createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) override;

        GLSupport& mGLSupport;
    };

	/** Handles creation of OpenGL textures. */
	class BS_RSGL_EXPORT GLTextureCoreManager : public TextureCoreManager
	{
	public:
		GLTextureCoreManager(GLSupport& support);

	protected:		
		/** @copydoc TextureCoreManager::createTextureInternal */
		SPtr<TextureCore> createTextureInternal(TextureType texType, UINT32 width, UINT32 height, UINT32 depth,
			int numMips, PixelFormat format, int usage = TU_DEFAULT, bool hwGammaCorrection = false,
			UINT32 multisampleCount = 0, UINT32 numArraySlices = 1, const SPtr<PixelData>& initialData = nullptr) override;

		/** @copydoc TextureCoreManager::createRenderTextureInternal */
		SPtr<RenderTextureCore> createRenderTextureInternal(const RENDER_TEXTURE_DESC_CORE& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		GLSupport& mGLSupport;
	};

	/** @} */
}
