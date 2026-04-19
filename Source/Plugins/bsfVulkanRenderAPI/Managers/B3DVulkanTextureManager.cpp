//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DVulkanTextureManager.h"

#include "B3DApplication.h"
#include "B3DVulkanTexture.h"
#include "Image/B3DTexture.h"
#include "B3DVulkanRenderTexture.h"
#include "B3DVulkanResource.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"

using namespace b3d;

struct DummyTexFormat
{
	TextureType Type;
	int ArraySize;
	int Width;
	int Height;
	int Depth;
};

const static DummyTexFormat DummyTexTypes[] = {
	{ TEX_TYPE_1D, 1, 2, 1, 1 },
	{ TEX_TYPE_1D, 2, 2, 1, 1 },
	{ TEX_TYPE_2D, 1, 2, 2, 1 },
	{ TEX_TYPE_2D, 2, 2, 2, 1 },
	{ TEX_TYPE_3D, 1, 2, 2, 2 },
	{ TEX_TYPE_CUBE_MAP, 1, 2, 2, 1 },
	{ TEX_TYPE_CUBE_MAP, 2, 2, 2, 1 }
};

SPtr<RenderTexture> VulkanTextureManager::CreateRenderTextureImpl(const RenderTextureCreateInformation& desc)
{
	VulkanRenderTexture* tex = new(B3DAllocate<VulkanRenderTexture>()) VulkanRenderTexture(desc);

	return B3DMakeSharedFromExisting<VulkanRenderTexture>(tex);
}

PixelFormat VulkanTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma)
{
	PixelUtility::CheckFormat(format, ttype, usage);

	if(render::VulkanUtility::GetPixelFormat(format, hwGamma) == VK_FORMAT_UNDEFINED)
		return PF_RGBA8;

	return format;
}

namespace b3d {
namespace render {
void VulkanTextureManager::OnStartUp()
{
	TextureManager::OnStartUp();

	int idx = 0;
	for(auto& entry : DummyTexTypes)
	{
		SPtr<PixelData> pixelData = PixelData::Create(entry.Width, entry.Height, entry.Depth, PF_RGBA8);

		for(int depth = 0; depth < entry.Depth; depth++)
			for(int height = 0; height < entry.Height; height++)
				for(int width = 0; width < entry.Width; width++)
					pixelData->SetColorAt(Color::kWhite, width, height, depth);

		TextureCreateInformation createInformation;
		createInformation.Type = entry.Type;
		createInformation.Width = entry.Width;
		createInformation.Height = entry.Height;
		createInformation.Depth = entry.Depth;
		createInformation.ArraySliceCount = entry.ArraySize;
		createInformation.Format = PF_RGBA8;
		createInformation.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::MutableFormat;

		createInformation.Name = "VulkanDummyRead";
		mDummyReadTextures[idx] = std::static_pointer_cast<VulkanTexture>(mGpuDevice.CreateTexture(createInformation));
		TextureUtility::Write(mDummyReadTextures[idx], *pixelData);

		createInformation.Name = "VulkanDummyStorage";
		createInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU | TextureUsageFlag::MutableFormat;
		mDummyStorageTextures[idx] = std::static_pointer_cast<VulkanTexture>(mGpuDevice.CreateTexture(createInformation));

		idx++;
	}
}

VulkanTexture* VulkanTextureManager::GetDummyTexture(GpuParameterObjectType type) const
{
	switch(type)
	{
	case GPOT_TEXTURE2DMS:
	case GPOT_TEXTURE2D:
		return mDummyReadTextures[2].get();
	case GPOT_RWTEXTURE2D:
	case GPOT_RWTEXTURE2DMS:
		return mDummyStorageTextures[2].get();
	case GPOT_TEXTURECUBE:
		return mDummyReadTextures[5].get();
	case GPOT_TEXTURECUBEARRAY:
		return mDummyReadTextures[6].get();
	case GPOT_TEXTURE2DARRAY:
	case GPOT_TEXTURE2DMSARRAY:
		return mDummyReadTextures[3].get();
	case GPOT_RWTEXTURE2DARRAY:
	case GPOT_RWTEXTURE2DMSARRAY:
		return mDummyStorageTextures[3].get();
	case GPOT_TEXTURE3D:
		return mDummyReadTextures[4].get();
	case GPOT_RWTEXTURE3D:
		return mDummyStorageTextures[4].get();
	case GPOT_TEXTURE1D:
		return mDummyReadTextures[0].get();
	case GPOT_TEXTURE1DARRAY:
		return mDummyReadTextures[1].get();
	case GPOT_RWTEXTURE1D:
		return mDummyStorageTextures[0].get();
	case GPOT_RWTEXTURE1DARRAY:
		return mDummyStorageTextures[1].get();
	default:
		return nullptr;
	}
}

VkFormat VulkanTextureManager::GetDummyViewFormat(GpuBufferFormat format)
{
	switch(format)
	{
	case BF_16X1F:
	case BF_32X1F:
	case BF_64X1F:
		return VK_FORMAT_R32_SFLOAT;
	case BF_16X2F:
	case BF_32X2F:
	case BF_64X2F:
		return VK_FORMAT_R16G16_UNORM;
	case BF_16X4F:
	case BF_32X3F:
	case BF_32X4F:
	case BF_64X3F:
	case BF_64X4F:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case BF_16X1U:
	case BF_32X1U:
	case BF_64X1U:
		return VK_FORMAT_R32_UINT;
	case BF_16X2U:
	case BF_32X2U:
	case BF_64X2U:
		return VK_FORMAT_R16G16_UINT;
	case BF_16X4U:
	case BF_32X3U:
	case BF_32X4U:
	case BF_64X3U:
	case BF_64X4U:
		return VK_FORMAT_R8G8B8A8_UINT;
	case BF_16X1S:
	case BF_32X1S:
	case BF_64X1S:
		return VK_FORMAT_R32_SINT;
	case BF_16X2S:
	case BF_32X2S:
	case BF_64X2S:
		return VK_FORMAT_R16G16_SINT;
	case BF_16X4S:
	case BF_32X3S:
	case BF_32X4S:
	case BF_64X3S:
	case BF_64X4S:
		return VK_FORMAT_R8G8B8A8_SINT;
	default:
		return VK_FORMAT_UNDEFINED;
	}
}

SPtr<RenderTexture> VulkanTextureManager::CreateRenderTextureInternal(const RenderTextureCreateInformation& desc)
{
	SPtr<VulkanRenderTexture> texPtr = B3DMakeShared<VulkanRenderTexture>(static_cast<VulkanGpuDevice&>(*GetApplication().GetPrimaryGpuDevice()), desc);
	texPtr->SetShared(texPtr);

	return texPtr;
}
}} // namespace b3d::render
