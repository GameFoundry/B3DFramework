//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRenderTexture.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "BsScriptTexture.generated.h"
#include "BsScriptRenderTexture.generated.h"
#include "../Extensions/BsRenderTargetEx.h"

namespace bs
{
	ScriptRenderTexture::ScriptRenderTexture(MonoObject* managedInstance, const SPtr<RenderTexture>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptRenderTexture::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptRenderTexture::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptRenderTexture::Internal_create0);
		metaData.scriptClass->addInternalCall("Internal_create1", (void*)&ScriptRenderTexture::Internal_create1);
		metaData.scriptClass->addInternalCall("Internal_create2", (void*)&ScriptRenderTexture::Internal_create2);
		metaData.scriptClass->addInternalCall("Internal_create3", (void*)&ScriptRenderTexture::Internal_create3);
		metaData.scriptClass->addInternalCall("Internal_getColorSurface", (void*)&ScriptRenderTexture::Internal_getColorSurface);
		metaData.scriptClass->addInternalCall("Internal_getColorSurfaces", (void*)&ScriptRenderTexture::Internal_getColorSurfaces);
		metaData.scriptClass->addInternalCall("Internal_getDepthStencilSurface", (void*)&ScriptRenderTexture::Internal_getDepthStencilSurface);

	}

	MonoObject* ScriptRenderTexture::create(const SPtr<RenderTexture>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, value);
		return managedInstance;
	}
	void ScriptRenderTexture::Internal_create(MonoObject* managedInstance, PixelFormat format, int32_t width, int32_t height, int32_t numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
	{
		SPtr<RenderTexture> instance = RenderTextureEx::create(format, width, height, numSamples, gammaCorrection, createDepth, depthStencilFormat);
		new (bs_alloc<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::Internal_create0(MonoObject* managedInstance, MonoObject* colorSurface)
	{
		ResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptcolorSurface;
		scriptcolorSurface = ScriptTexture::toNative(colorSurface);
		if(scriptcolorSurface != nullptr)
			tmpcolorSurface = scriptcolorSurface->getHandle();
		SPtr<RenderTexture> instance = RenderTextureEx::create(tmpcolorSurface);
		new (bs_alloc<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::Internal_create1(MonoObject* managedInstance, MonoObject* colorSurface, MonoObject* depthStencilSurface)
	{
		ResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptcolorSurface;
		scriptcolorSurface = ScriptTexture::toNative(colorSurface);
		if(scriptcolorSurface != nullptr)
			tmpcolorSurface = scriptcolorSurface->getHandle();
		ResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptdepthStencilSurface;
		scriptdepthStencilSurface = ScriptTexture::toNative(depthStencilSurface);
		if(scriptdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = scriptdepthStencilSurface->getHandle();
		SPtr<RenderTexture> instance = RenderTextureEx::create(tmpcolorSurface, tmpdepthStencilSurface);
		new (bs_alloc<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::Internal_create2(MonoObject* managedInstance, MonoArray* colorSurface)
	{
		Vector<ResourceHandle<Texture>> veccolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray arraycolorSurface(colorSurface);
			veccolorSurface.resize(arraycolorSurface.size());
			for(int i = 0; i < (int)arraycolorSurface.size(); i++)
			{
				ScriptTexture* scriptcolorSurface;
				scriptcolorSurface = ScriptTexture::toNative(arraycolorSurface.get<MonoObject*>(i));
				if(scriptcolorSurface != nullptr)
				{
					ResourceHandle<Texture> arrayElemPtrcolorSurface = scriptcolorSurface->getHandle();
					veccolorSurface[i] = arrayElemPtrcolorSurface;
				}
			}
		}
		SPtr<RenderTexture> instance = RenderTextureEx::create(veccolorSurface);
		new (bs_alloc<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::Internal_create3(MonoObject* managedInstance, MonoArray* colorSurface, MonoObject* depthStencilSurface)
	{
		Vector<ResourceHandle<Texture>> veccolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray arraycolorSurface(colorSurface);
			veccolorSurface.resize(arraycolorSurface.size());
			for(int i = 0; i < (int)arraycolorSurface.size(); i++)
			{
				ScriptTexture* scriptcolorSurface;
				scriptcolorSurface = ScriptTexture::toNative(arraycolorSurface.get<MonoObject*>(i));
				if(scriptcolorSurface != nullptr)
				{
					ResourceHandle<Texture> arrayElemPtrcolorSurface = scriptcolorSurface->getHandle();
					veccolorSurface[i] = arrayElemPtrcolorSurface;
				}
			}

		}
		ResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptdepthStencilSurface;
		scriptdepthStencilSurface = ScriptTexture::toNative(depthStencilSurface);
		if(scriptdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = scriptdepthStencilSurface->getHandle();
		SPtr<RenderTexture> instance = RenderTextureEx::create(veccolorSurface, tmpdepthStencilSurface);
		new (bs_alloc<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, instance);
	}

	MonoObject* ScriptRenderTexture::Internal_getColorSurface(ScriptRenderTexture* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = RenderTextureEx::getColorSurface(thisPtr->getInternal());

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoArray* ScriptRenderTexture::Internal_getColorSurfaces(ScriptRenderTexture* thisPtr)
	{
		Vector<ResourceHandle<Texture>> vec__output;
		vec__output = RenderTextureEx::getColorSurfaces(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptTexture>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptResourceBase* script__output;
			script__output = ScriptResourceManager::instance().getScriptResource(vec__output[i], true);
			if(script__output != nullptr)
				array__output.set(i, script__output->getManagedInstance());
			else
				array__output.set(i, nullptr);
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoObject* ScriptRenderTexture::Internal_getDepthStencilSurface(ScriptRenderTexture* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = RenderTextureEx::getDepthStencilSurface(thisPtr->getInternal());

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}
}
