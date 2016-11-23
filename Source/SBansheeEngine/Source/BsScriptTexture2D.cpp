//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptTexture2D.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsTexture.h"
#include "BsPixelUtil.h"
#include "BsException.h"
#include "BsScriptPixelData.h"
#include "BsScriptAsyncOp.h"
#include "BsCoreThread.h"

using namespace std::placeholders;

namespace bs
{
	ScriptTexture2D::ScriptTexture2D(MonoObject* instance, const HTexture& texture)
		:TScriptResource(instance, texture)
	{

	}

	void ScriptTexture2D::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptTexture2D::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPixels", &ScriptTexture2D::internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_GetGPUPixels", &ScriptTexture2D::internal_getGPUPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixels", &ScriptTexture2D::internal_setPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixelsArray", &ScriptTexture2D::internal_setPixelsArray);
	}

	void ScriptTexture2D::internal_createInstance(MonoObject* instance, PixelFormat format, UINT32 width,
		UINT32 height, TextureUsage usage, UINT32 numSamples, bool hasMipmaps, bool gammaCorrection)
	{
		int numMips = 0;
		if(hasMipmaps)
			numMips = PixelUtil::getMaxMipmaps(width, height, 1, format);

		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = width;
		texDesc.height = height;
		texDesc.numMips = numMips;
		texDesc.format = format;
		texDesc.usage = usage;
		texDesc.hwGamma = gammaCorrection;
		texDesc.numSamples = numSamples;

		HTexture texture = Texture::create(texDesc);

		ScriptTexture2D* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, texture, &scriptInstance);
	}

	MonoObject* ScriptTexture2D::internal_getPixels(ScriptTexture2D* thisPtr, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();

		SPtr<PixelData> pixelData = texture->getProperties().allocBuffer(0, mipLevel);
		texture->readCachedData(*pixelData, 0, mipLevel);

		return ScriptPixelData::create(pixelData);
	}

	MonoObject* ScriptTexture2D::internal_getGPUPixels(ScriptTexture2D* thisPtr, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();
		SPtr<PixelData> readData = texture->getProperties().allocBuffer(0, mipLevel);

		AsyncOp asyncOp = texture->readData(readData, 0, mipLevel);

		std::function<MonoObject*(const AsyncOp&, const SPtr<PixelData>&)> asyncOpToMono =
			[&](const AsyncOp& op, const SPtr<PixelData>& returnValue)
		{
			return ScriptPixelData::create(returnValue);
		};

		return ScriptAsyncOp::create(asyncOp, std::bind(asyncOpToMono, _1, readData));
	}

	void ScriptTexture2D::internal_setPixels(ScriptTexture2D* thisPtr, MonoObject* data, UINT32 mipLevel)
	{
		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(data);

		if (scriptPixelData != nullptr)
		{
			HTexture texture = thisPtr->getHandle();
			texture->writeData(scriptPixelData->getInternalValue(), 0, mipLevel, false);
		}
	}

	void ScriptTexture2D::internal_setPixelsArray(ScriptTexture2D* thisPtr, MonoArray* colors, UINT32 mipLevel)
	{
		ScriptArray scriptArray(colors);

		Color* colorsRaw = (Color*)scriptArray.getRawPtr(sizeof(Color), 0);
		UINT32 numElements = (UINT32)scriptArray.size();

		HTexture texture = thisPtr->getHandle();
		const TextureProperties& props = texture->getProperties();

		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(props.getWidth(), props.getHeight(), props.getDepth(), props.getFormat());
		pixelData->allocateInternalBuffer();
		pixelData->setColors(colorsRaw, numElements);

		texture->writeData(pixelData, 0, mipLevel, false);
	}

	MonoObject* ScriptTexture2D::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}