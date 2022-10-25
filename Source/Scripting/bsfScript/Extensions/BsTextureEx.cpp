//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsTextureEx.h"

#include "Generated/BsScriptPixelData.generated.h"

using namespace std::placeholders;

namespace bs
{
	HTexture TextureEx::Create(PixelFormat format, u32 width, u32 height, u32 depth, TextureType texType, TextureUsage usage, u32 numSamples, bool hasMipmaps, bool gammaCorrection)
	{
		int numMips = 0;
		if(hasMipmaps)
			numMips = PixelUtil::GetMaxMipmaps(width, height, 1, format);

		TEXTURE_DESC texDesc;
		texDesc.Type = texType;
		texDesc.Width = width;
		texDesc.Height = height;

		if(texType == TEX_TYPE_3D)
			texDesc.Depth = depth;
		else
			texDesc.Depth = 1;

		texDesc.NumMips = numMips;
		texDesc.Format = format;
		texDesc.Usage = usage;
		texDesc.HwGamma = gammaCorrection;
		texDesc.NumSamples = numSamples;

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

	u32 TextureEx::GetWidth(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetWidth();
	}

	u32 TextureEx::GetHeight(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetHeight();
	}

	u32 TextureEx::GetDepth(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetDepth();
	}

	bool TextureEx::GetGammaCorrection(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().IsHardwareGammaEnabled();
	}

	u32 TextureEx::GetSampleCount(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetNumSamples();
	}

	u32 TextureEx::GetMipmapCount(const HTexture& thisPtr)
	{
		return thisPtr->GetProperties().GetNumMipmaps();
	}

	SPtr<PixelData> TextureEx::GetPixels(const HTexture& thisPtr, u32 face, u32 mipLevel)
	{
		SPtr<PixelData> pixelData = thisPtr->GetProperties().AllocBuffer(face, mipLevel);
		thisPtr->ReadCachedData(*pixelData, face, mipLevel);

		return pixelData;
	}

	void TextureEx::SetPixels(const HTexture& thisPtr, const SPtr<PixelData>& data, u32 face, u32 mipLevel)
	{
		if(data != nullptr)
			thisPtr->WriteData(data, face, mipLevel, false);
	}

	void TextureEx::SetPixelsArray(const HTexture& thisPtr, const Vector<Color>& colors, u32 face, u32 mipLevel)
	{
		u32 numElements = (u32)colors.size();

		const TextureProperties& props = thisPtr->GetProperties();
		u32 texNumElements = props.GetWidth() * props.GetHeight() * props.GetDepth();

		if(texNumElements != numElements)
		{
			BS_LOG(Warning, Texture, "SetPixels called with incorrect dimensions. Ignoring call.");
			return;
		}

		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(props.GetWidth(), props.GetHeight(), props.GetDepth(), props.GetFormat());
		pixelData->AllocateInternalBuffer();
		pixelData->SetColors(colors);

		thisPtr->WriteData(pixelData, face, mipLevel, false);
	}
} // namespace bs
