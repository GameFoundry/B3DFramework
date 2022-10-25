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
		: TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptRenderTexture::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptRenderTexture::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptRenderTexture::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptRenderTexture::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptRenderTexture::InternalCreate2);
		metaData.ScriptClass->AddInternalCall("Internal_Create3", (void*)&ScriptRenderTexture::InternalCreate3);
		metaData.ScriptClass->AddInternalCall("Internal_GetColorSurface", (void*)&ScriptRenderTexture::InternalGetColorSurface);
		metaData.ScriptClass->AddInternalCall("Internal_GetColorSurfaces", (void*)&ScriptRenderTexture::InternalGetColorSurfaces);
		metaData.ScriptClass->AddInternalCall("Internal_GetDepthStencilSurface", (void*)&ScriptRenderTexture::InternalGetDepthStencilSurface);
	}

	MonoObject* ScriptRenderTexture::Create(const SPtr<RenderTexture>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, value);
		return managedInstance;
	}

	void ScriptRenderTexture::InternalCreate(MonoObject* managedInstance, PixelFormat format, int32_t width, int32_t height, int32_t numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
	{
		SPtr<RenderTexture> instance = RenderTextureEx::Create(format, width, height, numSamples, gammaCorrection, createDepth, depthStencilFormat);
		new(bs_alloc<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::InternalCreate0(MonoObject* managedInstance, MonoObject* colorSurface)
	{
		ResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptcolorSurface;
		scriptcolorSurface = ScriptTexture::ToNative(colorSurface);
		if(scriptcolorSurface != nullptr)
			tmpcolorSurface = scriptcolorSurface->GetHandle();
		SPtr<RenderTexture> instance = RenderTextureEx::Create(tmpcolorSurface);
		new(bs_alloc<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::InternalCreate1(MonoObject* managedInstance, MonoObject* colorSurface, MonoObject* depthStencilSurface)
	{
		ResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptcolorSurface;
		scriptcolorSurface = ScriptTexture::ToNative(colorSurface);
		if(scriptcolorSurface != nullptr)
			tmpcolorSurface = scriptcolorSurface->GetHandle();
		ResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptdepthStencilSurface;
		scriptdepthStencilSurface = ScriptTexture::ToNative(depthStencilSurface);
		if(scriptdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = scriptdepthStencilSurface->GetHandle();
		SPtr<RenderTexture> instance = RenderTextureEx::Create(tmpcolorSurface, tmpdepthStencilSurface);
		new(bs_alloc<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::InternalCreate2(MonoObject* managedInstance, MonoArray* colorSurface)
	{
		Vector<ResourceHandle<Texture>> veccolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray arraycolorSurface(colorSurface);
			veccolorSurface.resize(arraycolorSurface.Size());
			for(int i = 0; i < (int)arraycolorSurface.Size(); i++)
			{
				ScriptTexture* scriptcolorSurface;
				scriptcolorSurface = ScriptTexture::ToNative(arraycolorSurface.Get<MonoObject*>(i));
				if(scriptcolorSurface != nullptr)
				{
					ResourceHandle<Texture> arrayElemPtrcolorSurface = scriptcolorSurface->GetHandle();
					veccolorSurface[i] = arrayElemPtrcolorSurface;
				}
			}
		}
		SPtr<RenderTexture> instance = RenderTextureEx::Create(veccolorSurface);
		new(bs_alloc<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, instance);
	}

	void ScriptRenderTexture::InternalCreate3(MonoObject* managedInstance, MonoArray* colorSurface, MonoObject* depthStencilSurface)
	{
		Vector<ResourceHandle<Texture>> veccolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray arraycolorSurface(colorSurface);
			veccolorSurface.resize(arraycolorSurface.Size());
			for(int i = 0; i < (int)arraycolorSurface.Size(); i++)
			{
				ScriptTexture* scriptcolorSurface;
				scriptcolorSurface = ScriptTexture::ToNative(arraycolorSurface.Get<MonoObject*>(i));
				if(scriptcolorSurface != nullptr)
				{
					ResourceHandle<Texture> arrayElemPtrcolorSurface = scriptcolorSurface->GetHandle();
					veccolorSurface[i] = arrayElemPtrcolorSurface;
				}
			}
		}
		ResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptdepthStencilSurface;
		scriptdepthStencilSurface = ScriptTexture::ToNative(depthStencilSurface);
		if(scriptdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = scriptdepthStencilSurface->GetHandle();
		SPtr<RenderTexture> instance = RenderTextureEx::Create(veccolorSurface, tmpdepthStencilSurface);
		new(bs_alloc<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, instance);
	}

	MonoObject* ScriptRenderTexture::InternalGetColorSurface(ScriptRenderTexture* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = RenderTextureEx::GetColorSurface(thisPtr->GetInternal());

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoArray* ScriptRenderTexture::InternalGetColorSurfaces(ScriptRenderTexture* thisPtr)
	{
		Vector<ResourceHandle<Texture>> vec__output;
		vec__output = RenderTextureEx::GetColorSurfaces(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptTexture>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptResourceBase* script__output;
			script__output = ScriptResourceManager::Instance().GetScriptResource(vec__output[i], true);
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptRenderTexture::InternalGetDepthStencilSurface(ScriptRenderTexture* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = RenderTextureEx::GetDepthStencilSurface(thisPtr->GetInternal());

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}
} // namespace bs
