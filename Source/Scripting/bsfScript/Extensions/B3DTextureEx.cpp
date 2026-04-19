//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DTextureEx.h"

#include "Generated/B3DScriptPixelData.generated.h"

using namespace b3d;
HTexture TextureEx::Create(PixelFormat format, u32 width, u32 height, u32 depth, TextureType texType, TextureUsageFlags usage, u32 numSamples, bool hasMipmaps, bool gammaCorrection)
{
	int numMips = 0;
	if(hasMipmaps)
		numMips = PixelUtility::GetMipmapCount(width, height, 1, format);

	TextureCreateInformation texDesc;
	texDesc.Name = "Script Texture";
	texDesc.Type = texType;
	texDesc.Width = width;
	texDesc.Height = height;

	if(texType == TEX_TYPE_3D)
		texDesc.Depth = depth;
	else
		texDesc.Depth = 1;

	texDesc.MipMapCount = numMips;
	texDesc.Format = format;
	texDesc.Usage = usage;
	texDesc.UseHardwareSRGB = gammaCorrection;
	texDesc.SampleCount = numSamples;

	return Texture::Create(texDesc);
}

PixelFormat TextureEx::GetPixelFormat(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().Format;
}

TextureUsageFlags TextureEx::GetUsage(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().Usage;
}

TextureType TextureEx::GetType(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().Type;
}

u32 TextureEx::GetWidth(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().Width;
}

u32 TextureEx::GetHeight(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().Height;
}

u32 TextureEx::GetDepth(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().Depth;
}

bool TextureEx::GetGammaCorrection(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().UseHardwareSRGB;
}

u32 TextureEx::GetSampleCount(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().SampleCount;
}

u32 TextureEx::GetMipmapCount(const HTexture& thisPtr)
{
	return thisPtr->GetProperties().MipMapCount;
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
	u32 texNumElements = props.Width * props.Height * props.Depth;

	if(texNumElements != numElements)
	{
		B3D_LOG(Warning, LogTexture, "SetPixels called with incorrect dimensions. Ignoring call.");
		return;
	}

	SPtr<PixelData> pixelData = B3DMakeShared<PixelData>(props.Width, props.Height, props.Depth, props.Format);
	pixelData->AllocateInternalBuffer();
	pixelData->SetColors(colors);

	thisPtr->WriteData(pixelData, face, mipLevel, false);
}
