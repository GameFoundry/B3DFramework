//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptTexture.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptAsyncOp.h"

#include "BsScriptPixelData.generated.h"

using namespace std::placeholders;

namespace bs
{
	ScriptTexture::ScriptTexture(MonoObject* instance, const HTexture& texture)
		:TScriptResource(instance, texture)
	{

	}

	void ScriptTexture::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptTexture::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPixelFormat", &ScriptTexture::internal_getPixelFormat);
		metaData.scriptClass->addInternalCall("Internal_GetUsage", &ScriptTexture::internal_getUsage);
		metaData.scriptClass->addInternalCall("Internal_GetWidth", &ScriptTexture::internal_getWidth);
		metaData.scriptClass->addInternalCall("Internal_GetHeight", &ScriptTexture::internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_GetGammaCorrection", &ScriptTexture::internal_getGammaCorrection);
		metaData.scriptClass->addInternalCall("Internal_GetSampleCount", &ScriptTexture::internal_getSampleCount);
		metaData.scriptClass->addInternalCall("Internal_GetMipmapCount", &ScriptTexture::internal_getMipmapCount);
		metaData.scriptClass->addInternalCall("Internal_GetPixels", &ScriptTexture::internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_GetGPUPixels", &ScriptTexture::internal_getGPUPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixels", &ScriptTexture::internal_setPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixelsArray", &ScriptTexture::internal_setPixelsArray);
	}

	void ScriptTexture::internal_createInstance(MonoObject* instance, PixelFormat format, UINT32 width,
		UINT32 height, UINT32 depth, TextureType texType, TextureUsage usage, UINT32 numSamples, bool hasMipmaps, 
		bool gammaCorrection)
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

		HTexture texture = Texture::create(texDesc);

		ScriptTexture* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, texture, &scriptInstance);
	}

	void ScriptTexture::internal_getPixelFormat(ScriptTexture* thisPtr, PixelFormat* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getGenericHandle());
		*value = texture->getProperties().getFormat();
	}

	void ScriptTexture::internal_getUsage(ScriptTexture* thisPtr, TextureUsage* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getGenericHandle());
		*value = (TextureUsage)texture->getProperties().getUsage();
	}

	void ScriptTexture::internal_getWidth(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getGenericHandle());
		*value = (TextureUsage)texture->getProperties().getWidth();
	}

	void ScriptTexture::internal_getHeight(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getGenericHandle());
		*value = (TextureUsage)texture->getProperties().getHeight();
	}

	void ScriptTexture::internal_getGammaCorrection(ScriptTexture* thisPtr, bool* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getGenericHandle());
		*value = (TextureUsage)texture->getProperties().isHardwareGammaEnabled();
	}

	void ScriptTexture::internal_getSampleCount(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getGenericHandle());
		*value = (TextureUsage)texture->getProperties().getNumSamples();
	}

	void ScriptTexture::internal_getMipmapCount(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getGenericHandle());
		*value = (TextureUsage)texture->getProperties().getNumMipmaps();
	}

	MonoObject* ScriptTexture::internal_getPixels(ScriptTexture* thisPtr, UINT32 face, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();

		SPtr<PixelData> pixelData = texture->getProperties().allocBuffer(face, mipLevel);
		texture->readCachedData(*pixelData, face, mipLevel);

		return ScriptPixelData::create(pixelData);
	}

	MonoObject* ScriptTexture::internal_getGPUPixels(ScriptTexture* thisPtr, UINT32 face, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();
		SPtr<PixelData> readData = texture->getProperties().allocBuffer(face, mipLevel);

		AsyncOp asyncOp = texture->readData(readData, face, mipLevel);

		std::function<MonoObject*(const AsyncOp&, const SPtr<PixelData>&)> asyncOpToMono =
			[&](const AsyncOp& op, const SPtr<PixelData>& returnValue)
		{
			return ScriptPixelData::create(returnValue);
		};

		return ScriptAsyncOp::create(asyncOp, std::bind(asyncOpToMono, _1, readData));
	}

	void ScriptTexture::internal_setPixels(ScriptTexture* thisPtr, MonoObject* data, UINT32 face, UINT32 mipLevel)
	{
		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(data);

		if (scriptPixelData != nullptr)
		{
			HTexture texture = thisPtr->getHandle();
			texture->writeData(scriptPixelData->getInternal(), face, mipLevel, false);
		}
	}

	void ScriptTexture::internal_setPixelsArray(ScriptTexture* thisPtr, MonoArray* colors, UINT32 face, UINT32 mipLevel)
	{
		if (colors == nullptr)
		{
			LOGWRN("SetPixels called with a null array. Ignoring call.");
			return;
		}

		ScriptArray scriptArray(colors);
		Color* colorsRaw = (Color*)scriptArray.getRawPtr(sizeof(Color), 0);
		UINT32 numElements = (UINT32)scriptArray.size();

		HTexture texture = thisPtr->getHandle();
		const TextureProperties& props = texture->getProperties();

		UINT32 texNumElements = props.getWidth() * props.getHeight() * props.getDepth();

		if (texNumElements != numElements)
		{
			LOGWRN("SetPixels called with incorrect dimensions. Ignoring call.");
			return;
		}		

		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(props.getWidth(), props.getHeight(), props.getDepth(), 
			props.getFormat());
		pixelData->allocateInternalBuffer();
		pixelData->setColors(colorsRaw, numElements);

		texture->writeData(pixelData, face, mipLevel, false);
	}


	MonoObject* ScriptTexture::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}