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
	GLRenderTexture* tex = new(B3DAllocate<GLRenderTexture>()) GLRenderTexture(desc);

	return B3DMakeCoreFromExisting<GLRenderTexture>(tex);
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
	GLTexture* tex = new(B3DAllocate<GLTexture>()) GLTexture(mGLSupport, desc, initialData, deviceMask);

	SPtr<GLTexture> texPtr = B3DMakeSharedFromExisting<GLTexture>(tex);
	texPtr->SetThisPtrInternal(texPtr);

	return texPtr;
}

SPtr<RenderTexture> GLTextureManager::CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
{
	SPtr<GLRenderTexture> texPtr = B3DMakeShared<GLRenderTexture>(desc, deviceIdx);
	texPtr->SetThisPtrInternal(texPtr);

	return texPtr;
}
}} // namespace bs::ct
