//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullTexture.h"
#include "BsNullRenderTargets.h"

using namespace b3d;

SPtr<RenderTexture> NullTextureManager::CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
{
	return B3DMakeCoreShared<NullRenderTexture>(desc);
}

PixelFormat NullTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
{
	return format;
}

namespace b3d { namespace render {
SPtr<Texture> NullTextureManager::CreateTextureInternal(const TextureCreateInformation& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
{
	SPtr<NullTexture> texPtr = B3DMakeShared<NullTexture>(desc, initialData, deviceMask);
	texPtr->SetShared(texPtr);

	return texPtr;
}

SPtr<RenderTexture> NullTextureManager::CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
{
	SPtr<NullRenderTexture> texPtr = B3DMakeShared<NullRenderTexture>(desc, deviceIdx);
	texPtr->SetShared(texPtr);

	return texPtr;
}

NullTexture::NullTexture(const TextureCreateInformation& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	: Texture(desc, initialData, deviceMask)
{}

NullTexture::~NullTexture()
{
	ClearBufferViews();
}

PixelData NullTexture::LockInternal(GpuLockOptions options, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	u32 mipWidth = std::max(1u, mProperties.Width >> mipLevel);
	u32 mipHeight = std::max(1u, mProperties.Height >> mipLevel);
	u32 mipDepth = std::max(1u, mProperties.Depth >> mipLevel);

	mMappedBuffer = B3DNew<PixelData>(mipWidth, mipHeight, mipDepth, mProperties.Format);
	mMappedBuffer->AllocateInternalBuffer();

	PixelData output(mipWidth, mipHeight, mipDepth, mProperties.Format);
	output.SetExternalBuffer(mMappedBuffer->GetData());

	return output;
}

void NullTexture::UnlockInternal()
{
	B3DDelete(mMappedBuffer);
	mMappedBuffer = nullptr;
}
}} // namespace b3d::render
