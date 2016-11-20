//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLTextureManager.h"
#include "BsRenderAPI.h"
#include "BsGLRenderTexture.h"
#include "BsGLPixelFormat.h"

namespace bs
{
    GLTextureManager::GLTextureManager(GLSupport& support)
        :TextureManager(), mGLSupport(support)
    {

    }

    GLTextureManager::~GLTextureManager()
    {

    }

	SPtr<RenderTexture> GLTextureManager::createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
	{
		GLRenderTexture* tex = new (bs_alloc<GLRenderTexture>()) GLRenderTexture(desc);

		return bs_core_ptr<GLRenderTexture>(tex);
	}

	PixelFormat GLTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
        // Check if this is a valid rendertarget format
		if(usage & TU_RENDERTARGET)
            return GLRTTManager::instance().getSupportedAlternative(format);

		return GLPixelUtil::getClosestValidFormat(format);
	}

	GLTextureCoreManager::GLTextureCoreManager(GLSupport& support)
		:mGLSupport(support)
	{ }

	SPtr<TextureCore> GLTextureCoreManager::createTextureInternal(const TEXTURE_DESC& desc,
		const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	{
		GLTextureCore* tex = new (bs_alloc<GLTextureCore>()) GLTextureCore(mGLSupport, desc, initialData, deviceMask);

		SPtr<GLTextureCore> texPtr = bs_shared_ptr<GLTextureCore>(tex);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<RenderTextureCore> GLTextureCoreManager::createRenderTextureInternal(const RENDER_TEXTURE_DESC_CORE& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<GLRenderTextureCore> texPtr = bs_shared_ptr_new<GLRenderTextureCore>(desc, deviceMask);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}
}
