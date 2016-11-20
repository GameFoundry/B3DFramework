//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptTextureCube.h"
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
	ScriptTextureCube::ScriptTextureCube(MonoObject* instance, const HTexture& texture)
		:TScriptResource(instance, texture)
	{

	}

	void ScriptTextureCube::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptTextureCube::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPixels", &ScriptTextureCube::internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_GetGPUPixels", &ScriptTextureCube::internal_getGPUPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixels", &ScriptTextureCube::internal_setPixels);
	}

	void ScriptTextureCube::internal_createInstance(MonoObject* instance, PixelFormat format, UINT32 width,
		UINT32 height, TextureUsage usage, UINT32 numSamples, bool hasMipmaps, bool gammaCorrection)
	{
		int numMips = 0;
		if (hasMipmaps)
			numMips = PixelUtil::getMaxMipmaps(width, height, 1, format);

		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_CUBE_MAP;
		texDesc.width = width;
		texDesc.height = height;
		texDesc.numMips = numMips;
		texDesc.format = format;
		texDesc.usage = usage;
		texDesc.hwGamma = gammaCorrection;
		texDesc.numSamples = numSamples;

		HTexture texture = Texture::create(texDesc);

		ScriptTextureCube* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, texture, &scriptInstance);
	}

	MonoObject* ScriptTextureCube::internal_getPixels(ScriptTextureCube* thisPtr, UINT32 face, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();
		UINT32 subresourceIdx = texture->getProperties().mapToSubresourceIdx(face, mipLevel);

		SPtr<PixelData> pixelData = texture->getProperties().allocateSubresourceBuffer(subresourceIdx);

		texture->readData(*pixelData, mipLevel, face);

		return ScriptPixelData::create(pixelData);
	}

	MonoObject* ScriptTextureCube::internal_getGPUPixels(ScriptTextureCube* thisPtr, UINT32 face, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();
		UINT32 subresourceIdx = texture->getProperties().mapToSubresourceIdx(face, mipLevel);

		SPtr<PixelData> readData = texture->getProperties().allocateSubresourceBuffer(subresourceIdx);
		AsyncOp asyncOp = texture->readSubresource(gCoreAccessor(), subresourceIdx, readData);

		std::function<MonoObject*(const AsyncOp&, const SPtr<PixelData>&)> asyncOpToMono =
			[&](const AsyncOp& op, const SPtr<PixelData>& returnValue)
		{
			return ScriptPixelData::create(returnValue);
		};

		return ScriptAsyncOp::create(asyncOp, std::bind(asyncOpToMono, _1, readData));
	}

	void ScriptTextureCube::internal_setPixels(ScriptTextureCube* thisPtr, MonoObject* data, UINT32 face, UINT32 mipLevel)
	{
		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(data);

		if (scriptPixelData != nullptr)
		{
			HTexture texture = thisPtr->getHandle();
			UINT32 subresourceIdx = texture->getProperties().mapToSubresourceIdx(face, mipLevel);

			texture->writeSubresource(gCoreAccessor(), subresourceIdx, scriptPixelData->getInternalValue(), false);
		}
	}

	MonoObject* ScriptTextureCube::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}