//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Extensions/BsTextureEx.h"
#include "Extensions/BsAsyncOpEx.h"

#include "BsScriptPixelData.generated.h"

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

		return Texture::create(texDesc);
	}

	PixelFormat TextureEx::getPixelFormat(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().getFormat();
	}

	TextureUsage TextureEx::getUsage(const HTexture& thisPtr)
	{
		return (TextureUsage)thisPtr->getProperties().getUsage();
	}

	TextureType TextureEx::getType(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().getTextureType();
	}

	UINT32 TextureEx::getWidth(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().getWidth();
	}

	UINT32 TextureEx::getHeight(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().getHeight();
	}

	UINT32 TextureEx::getDepth(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().getDepth();
	}

	bool TextureEx::getGammaCorrection(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().isHardwareGammaEnabled();
	}

	UINT32 TextureEx::getSampleCount(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().getNumSamples();
	}

	UINT32 TextureEx::getMipmapCount(const HTexture& thisPtr)
	{
		return thisPtr->getProperties().getNumMipmaps();
	}

	SPtr<PixelData> TextureEx::getPixels(const HTexture& thisPtr, UINT32 face, UINT32 mipLevel)
	{
		SPtr<PixelData> pixelData = thisPtr->getProperties().allocBuffer(face, mipLevel);
		thisPtr->readCachedData(*pixelData, face, mipLevel);

		return pixelData;
	}

	SPtr<AsyncOpEx> TextureEx::getGPUPixels(const HTexture& thisPtr, UINT32 face, UINT32 mipLevel)
	{
		SPtr<PixelData> readData = thisPtr->getProperties().allocBuffer(face, mipLevel);
		AsyncOp asyncOp = thisPtr->readData(readData, face, mipLevel);

		std::function<MonoObject*(const AsyncOp&)> asyncOpToMono =
			[&readData](const AsyncOp& op)
		{
			return ScriptPixelData::create(readData);
		};

		return bs_shared_ptr_new<AsyncOpEx>(asyncOp, asyncOpToMono);
	}

	void TextureEx::setPixels(const HTexture& thisPtr, const SPtr<PixelData>& data, UINT32 face, UINT32 mipLevel)
	{
		if (data != nullptr)
			thisPtr->writeData(data, face, mipLevel, false);
	}

	void TextureEx::setPixelsArray(const HTexture& thisPtr, const Vector<Color>& colors, UINT32 face, UINT32 mipLevel)
	{
		UINT32 numElements = (UINT32)colors.size();

		const TextureProperties& props = thisPtr->getProperties();
		UINT32 texNumElements = props.getWidth() * props.getHeight() * props.getDepth();

		if (texNumElements != numElements)
		{
			LOGWRN("SetPixels called with incorrect dimensions. Ignoring call.");
			return;
		}		

		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(props.getWidth(), props.getHeight(), props.getDepth(), 
			props.getFormat());
		pixelData->allocateInternalBuffer();
		pixelData->setColors(colors);

		thisPtr->writeData(pixelData, face, mipLevel, false);
	}
}