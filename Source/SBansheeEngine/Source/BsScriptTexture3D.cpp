//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptTexture3D.h"
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
	ScriptTexture3D::ScriptTexture3D(MonoObject* instance, const HTexture& texture)
		:TScriptResource(instance, texture)
	{

	}

	void ScriptTexture3D::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptTexture3D::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPixels", &ScriptTexture3D::internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_GetGPUPixels", &ScriptTexture3D::internal_getGPUPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixels", &ScriptTexture3D::internal_setPixels);
	}

	void ScriptTexture3D::internal_createInstance(MonoObject* instance, PixelFormat format, UINT32 width,
		UINT32 height, UINT32 depth, TextureUsage usage, bool hasMipmaps, bool gammaCorrection)
	{
		int numMips = 0;
		if (hasMipmaps)
			numMips = PixelUtil::getMaxMipmaps(width, height, depth, format);

		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_3D;
		texDesc.width = width;
		texDesc.height = height;
		texDesc.depth = depth;
		texDesc.numMips = numMips;
		texDesc.format = format;
		texDesc.usage = usage;
		texDesc.hwGamma = gammaCorrection;

		HTexture texture = Texture::create(texDesc);

		ScriptTexture3D* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, texture, &scriptInstance);
	}

	MonoObject* ScriptTexture3D::internal_getPixels(ScriptTexture3D* thisPtr, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();
		UINT32 subresourceIdx = texture->getProperties().mapToSubresourceIdx(0, mipLevel);

		SPtr<PixelData> pixelData = texture->getProperties().allocateSubresourceBuffer(subresourceIdx);

		texture->readData(*pixelData, mipLevel);

		return ScriptPixelData::create(pixelData);
	}

	MonoObject* ScriptTexture3D::internal_getGPUPixels(ScriptTexture3D* thisPtr, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->getHandle();
		UINT32 subresourceIdx = texture->getProperties().mapToSubresourceIdx(0, mipLevel);

		SPtr<PixelData> readData = texture->getProperties().allocateSubresourceBuffer(subresourceIdx);

		AsyncOp asyncOp = texture->readSubresource(gCoreAccessor(), subresourceIdx, readData);

		std::function<MonoObject*(const AsyncOp&, const SPtr<PixelData>&)> asyncOpToMono =
			[&](const AsyncOp& op, const SPtr<PixelData>& returnValue)
		{
			return ScriptPixelData::create(returnValue);
		};

		return ScriptAsyncOp::create(asyncOp, std::bind(asyncOpToMono, _1, readData));
	}

	void ScriptTexture3D::internal_setPixels(ScriptTexture3D* thisPtr, MonoObject* data, UINT32 mipLevel)
	{
		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(data);

		if (scriptPixelData != nullptr)
		{
			HTexture texture = thisPtr->getHandle();
			UINT32 subresourceIdx = texture->getProperties().mapToSubresourceIdx(0, mipLevel);

			texture->writeSubresource(gCoreAccessor(), subresourceIdx, scriptPixelData->getInternalValue(), false);
		}
	}

	MonoObject* ScriptTexture3D::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}