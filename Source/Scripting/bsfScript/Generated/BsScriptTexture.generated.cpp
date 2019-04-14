//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTexture.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptAsyncOp.h"
#include "BsScriptPixelData.generated.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "Wrappers/BsScriptColor.h"
#include "../Extensions/BsTextureEx.h"

namespace bs
{
	ScriptTexture::ScriptTexture(MonoObject* managedInstance, const ResourceHandle<Texture>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptTexture::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRef", (void*)&ScriptTexture::Internal_getRef);
		metaData.scriptClass->addInternalCall("Internal_readData", (void*)&ScriptTexture::Internal_readData);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptTexture::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_getPixelFormat", (void*)&ScriptTexture::Internal_getPixelFormat);
		metaData.scriptClass->addInternalCall("Internal_getUsage", (void*)&ScriptTexture::Internal_getUsage);
		metaData.scriptClass->addInternalCall("Internal_getType", (void*)&ScriptTexture::Internal_getType);
		metaData.scriptClass->addInternalCall("Internal_getWidth", (void*)&ScriptTexture::Internal_getWidth);
		metaData.scriptClass->addInternalCall("Internal_getHeight", (void*)&ScriptTexture::Internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_getDepth", (void*)&ScriptTexture::Internal_getDepth);
		metaData.scriptClass->addInternalCall("Internal_getGammaCorrection", (void*)&ScriptTexture::Internal_getGammaCorrection);
		metaData.scriptClass->addInternalCall("Internal_getSampleCount", (void*)&ScriptTexture::Internal_getSampleCount);
		metaData.scriptClass->addInternalCall("Internal_getMipmapCount", (void*)&ScriptTexture::Internal_getMipmapCount);
		metaData.scriptClass->addInternalCall("Internal_getPixels", (void*)&ScriptTexture::Internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_setPixels", (void*)&ScriptTexture::Internal_setPixels);
		metaData.scriptClass->addInternalCall("Internal_setPixelsArray", (void*)&ScriptTexture::Internal_setPixelsArray);

	}

	 MonoObject*ScriptTexture::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->createInstance("bool", ctorParams);
	}
	MonoObject* ScriptTexture::Internal_getRef(ScriptTexture* thisPtr)
	{
		return thisPtr->getRRef();
	}

	MonoObject* ScriptTexture::Internal_readData(ScriptTexture* thisPtr, uint32_t face, uint32_t mipLevel)
	{
		TAsyncOp<SPtr<PixelData>> tmp__output;
		tmp__output = thisPtr->getHandle()->readData(face, mipLevel);

		MonoObject* __output;
		auto convertCallback = [](const Any& returnVal)
		{
			SPtr<PixelData> nativeObj = any_cast<SPtr<PixelData>>(returnVal);
			MonoObject* monoObj;
			monoObj = ScriptPixelData::create(nativeObj);
			return monoObj;
		};

;		__output = ScriptAsyncOpBase::create(tmp__output, convertCallback, ScriptPixelData::getMetaData()->scriptClass);

		return __output;
	}

	void ScriptTexture::Internal_create(MonoObject* managedInstance, PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, TextureType texType, TextureUsage usage, uint32_t numSamples, bool hasMipmaps, bool gammaCorrection)
	{
		ResourceHandle<Texture> instance = TextureEx::create(format, width, height, depth, texType, usage, numSamples, hasMipmaps, gammaCorrection);
		ScriptResourceManager::instance().createBuiltinScriptResource(instance, managedInstance);
	}

	PixelFormat ScriptTexture::Internal_getPixelFormat(ScriptTexture* thisPtr)
	{
		PixelFormat tmp__output;
		tmp__output = TextureEx::getPixelFormat(thisPtr->getHandle());

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	TextureUsage ScriptTexture::Internal_getUsage(ScriptTexture* thisPtr)
	{
		TextureUsage tmp__output;
		tmp__output = TextureEx::getUsage(thisPtr->getHandle());

		TextureUsage __output;
		__output = tmp__output;

		return __output;
	}

	TextureType ScriptTexture::Internal_getType(ScriptTexture* thisPtr)
	{
		TextureType tmp__output;
		tmp__output = TextureEx::getType(thisPtr->getHandle());

		TextureType __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::Internal_getWidth(ScriptTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::getWidth(thisPtr->getHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::Internal_getHeight(ScriptTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::getHeight(thisPtr->getHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::Internal_getDepth(ScriptTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::getDepth(thisPtr->getHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptTexture::Internal_getGammaCorrection(ScriptTexture* thisPtr)
	{
		bool tmp__output;
		tmp__output = TextureEx::getGammaCorrection(thisPtr->getHandle());

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::Internal_getSampleCount(ScriptTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::getSampleCount(thisPtr->getHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::Internal_getMipmapCount(ScriptTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::getMipmapCount(thisPtr->getHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptTexture::Internal_getPixels(ScriptTexture* thisPtr, uint32_t face, uint32_t mipLevel)
	{
		SPtr<PixelData> tmp__output;
		tmp__output = TextureEx::getPixels(thisPtr->getHandle(), face, mipLevel);

		MonoObject* __output;
		__output = ScriptPixelData::create(tmp__output);

		return __output;
	}

	void ScriptTexture::Internal_setPixels(ScriptTexture* thisPtr, MonoObject* data, uint32_t face, uint32_t mipLevel)
	{
		SPtr<PixelData> tmpdata;
		ScriptPixelData* scriptdata;
		scriptdata = ScriptPixelData::toNative(data);
		if(scriptdata != nullptr)
			tmpdata = scriptdata->getInternal();
		TextureEx::setPixels(thisPtr->getHandle(), tmpdata, face, mipLevel);
	}

	void ScriptTexture::Internal_setPixelsArray(ScriptTexture* thisPtr, MonoArray* colors, uint32_t face, uint32_t mipLevel)
	{
		Vector<Color> veccolors;
		if(colors != nullptr)
		{
			ScriptArray arraycolors(colors);
			veccolors.resize(arraycolors.size());
			for(int i = 0; i < (int)arraycolors.size(); i++)
			{
				veccolors[i] = arraycolors.get<Color>(i);
			}

		}
		TextureEx::setPixelsArray(thisPtr->getHandle(), veccolors, face, mipLevel);
	}
}
