//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLTextureManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "BsGLRenderTexture.h"
#include "BsGLPixelFormat.h"

using namespace bs;

GLTextureManager::GLTextureManager(ct::GLSupport& support)
	: TextureManager(), mGLSupport(support)
{
}

SPtr<RenderTexture> GLTextureManager::CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
{
	GLRenderTexture* tex = new(bs_alloc<GLRenderTexture>()) GLRenderTexture(desc);

	return bs_core_ptr<GLRenderTexture>(tex);
}

PixelFormat GLTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
{
	// Check if this is a valid rendertarget format
	if(usage & TU_RENDERTARGET)
		return ct::GLRTTManager::Instance().GetSupportedAlternative(format);

	return ct::GLPixelUtil::GetClosestSupportedPf(format, ttype, usage);
}

namespace bs { namespace ct {
GLTextureManager::GLTextureManager(GLSupport& support)
	: mGLSupport(support)
{}

SPtr<Texture> GLTextureManager::CreateTextureInternal(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
{
	GLTexture* tex = new(bs_alloc<GLTexture>()) GLTexture(mGLSupport, desc, initialData, deviceMask);

	SPtr<GLTexture> texPtr = bs_shared_ptr<GLTexture>(tex);
	texPtr->SetThisPtrInternal(texPtr);

	return texPtr;
}

SPtr<RenderTexture> GLTextureManager::CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
{
	SPtr<GLRenderTexture> texPtr = bs_shared_ptr_new<GLRenderTexture>(desc, deviceIdx);
	texPtr->SetThisPtrInternal(texPtr);

	return texPtr;
}
}} // namespace bs::ct
