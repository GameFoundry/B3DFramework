//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullTexture.h"
#include "BsNullRenderTargets.h"

using namespace bs;

SPtr<RenderTexture> NullTextureManager::CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
{
	return B3DMakeCoreShared<NullRenderTexture>(desc);
}

PixelFormat NullTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
{
	return format;
}

namespace bs { namespace ct {
SPtr<Texture> NullTextureManager::CreateTextureInternal(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
{
	SPtr<NullTexture> texPtr = B3DMakeShared<NullTexture>(desc, initialData, deviceMask);
	texPtr->SetThisPtrInternal(texPtr);

	return texPtr;
}

SPtr<RenderTexture> NullTextureManager::CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
{
	SPtr<NullRenderTexture> texPtr = B3DMakeShared<NullRenderTexture>(desc, deviceIdx);
	texPtr->SetThisPtrInternal(texPtr);

	return texPtr;
}

NullTexture::NullTexture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	: Texture(desc, initialData, deviceMask)
{}

NullTexture::~NullTexture()
{
	ClearBufferViews();
}

PixelData NullTexture::LockImpl(GpuLockOptions options, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	u32 mipWidth = std::max(1u, mProperties.GetWidth() >> mipLevel);
	u32 mipHeight = std::max(1u, mProperties.GetHeight() >> mipLevel);
	u32 mipDepth = std::max(1u, mProperties.GetDepth() >> mipLevel);

	mMappedBuffer = B3DNew<PixelData>(mipWidth, mipHeight, mipDepth, mProperties.GetFormat());
	mMappedBuffer->AllocateInternalBuffer();

	PixelData output(mipWidth, mipHeight, mipDepth, mProperties.GetFormat());
	output.SetExternalBuffer(mMappedBuffer->GetData());

	return output;
}

void NullTexture::UnlockImpl()
{
	B3DDelete(mMappedBuffer);
	mMappedBuffer = nullptr;
}
}} // namespace bs::ct
