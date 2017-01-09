//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLTextureManager.h"
#include "BsRenderAPI.h"
#include "BsGLRenderTexture.h"
#include "BsGLPixelFormat.h"

namespace bs
{
    GLTextureManager::GLTextureManager(ct::GLSupport& support)
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
            return ct::GLRTTManager::instance().getSupportedAlternative(format);

		return ct::GLPixelUtil::getClosestSupportedPF(format, ttype, usage);
	}

	namespace ct
	{
	GLTextureManager::GLTextureManager(GLSupport& support)
		:mGLSupport(support)
	{ }

	SPtr<TextureCore> GLTextureManager::createTextureInternal(const TEXTURE_DESC& desc,
		const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	{
		GLTexture* tex = new (bs_alloc<GLTexture>()) GLTexture(mGLSupport, desc, initialData, deviceMask);

		SPtr<GLTexture> texPtr = bs_shared_ptr<GLTexture>(tex);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<RenderTextureCore> GLTextureManager::createRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, 
																			  UINT32 deviceIdx)
	{
		SPtr<GLRenderTexture> texPtr = bs_shared_ptr_new<GLRenderTexture>(desc, deviceIdx);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}
		}
}