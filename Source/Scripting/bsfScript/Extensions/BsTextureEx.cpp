//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsTextureEx.h"

#include "Generated/BsScriptPixelData.generated.h"

using namespace std::placeholders;

namespace bs
{
	HTexture TextureEx::create(PixelFormat format, UINT32 width, UINT32 height, UINT32 depth, TextureType texType,
		TextureUsage usage, UINT32 numSamples, bool hasMipmaps, bool gammaCorrection)
	{
		int numMips = 0;
		if (hasMipmaps)
			numMips = PixelUtil::getMaxMipmaps(width, height, 1, format);

		TEXTURE_DESC texDesc;
		texDesc.type = texType;
		texDesc.width = width;
		texDesc.height = height;
		
		if (texType == TEX_TYPE_3D)
			texDesc.depth = depth;
		else
			texDesc.depth = 1;

		texDesc.numMips = numMips;
		texDesc.format = format;
		texDesc.usage = usage;
		texDesc.hwGamma = gammaCorrection;
		texDesc.numSamples = numSamples;

		return Texture::Create(texDesc);
	}

	PixelFormat TextureEx::GetPixelFormat(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetFormat();
	}

	TextureUsage TextureEx::GetUsage(const HTexture& thisPtr)
	{
		return (TextureUsage)thisPtr->GetProperties().GetUsage();
	}

	TextureType TextureEx::GetType(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetTextureType();
	}

	UINT32 TextureEx::GetWidth(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetWidth();
	}

	UINT32 TextureEx::GetHeight(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetHeight();
	}

	UINT32 TextureEx::GetDepth(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetDepth();
	}

	bool TextureEx::GetGammaCorrection(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().IsHardwareGammaEnabled();
	}

	UINT32 TextureEx::GetSampleCount(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetNumSamples();
	}

	UINT32 TextureEx::GetMipmapCount(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetNumMipmaps();
	}

	SPtr<PixelData> TextureEx::GetPixels(const HTexture& thisPtr, UINT32 face, UINT32 mipLevel)
	{
		SPtr<PixelData> pixelData = thisPtr->GetProperties().AllocBuffer(face, mipLevel);
		thisPtr->ReadCachedData(*pixelData, face, mipLevel);

		return pixelData;
	}

	void TextureEx::SetPixels(const HTexture& thisPtr, const SPtr<PixelData>& data, UINT32 face, UINT32 mipLevel)
	{
		if (data != nullptr)
			thisPtr->WriteData(data, face, mipLevel, false);
	}

	void TextureEx::SetPixelsArray(const HTexture& thisPtr, const Vector<Color>& colors, UINT32 face, UINT32 mipLevel)
	{
		UINT32 numElements = (UINT32)colors.Size();

		const TextureProperties& props = thisPtr->GetProperties();
		UINT32 texNumElements = props.GetWidth() * props.getHeight() * props.getDepth();

		if (texNumElements != numElements)
		{
			BS_LOG(Warning, Texture, "SetPixels called with incorrect dimensions. Ignoring call.");
			return;
		}		

		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(props.GetWidth(), props.getHeight(), props.getDepth(),
			props.GetFormat());
		pixelData->AllocateInternalBuffer();
		pixelData->SetColors(colors);

		thisPtr->WriteData(pixelData, face, mipLevel, false);
	}
}
