//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		new (B3DAllocate<ScriptRenderTexture>()) ScriptRenderTexture(managedInstance, value);
		return managedInstance;
	}
	void ScriptRenderTexture::InternalCreate(MonoObject* managedInstance, PixelFormat format, int32_t width, int32_t height, int32_t numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
	{
		SPtr<RenderTexture> nativeObject = RenderTextureEx::Create(format, width, height, numSamples, gammaCorrection, createDepth, depthStencilFormat);
		new (B3DAllocate<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, nativeObject);
	}

	void ScriptRenderTexture::InternalCreate0(MonoObject* managedInstance, MonoObject* colorSurface)
	{
		TResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptObjectWrappercolorSurface;
		scriptObjectWrappercolorSurface = ScriptTexture::ToNative(colorSurface);
		if(scriptObjectWrappercolorSurface != nullptr)
			tmpcolorSurface = scriptObjectWrappercolorSurface->GetHandle();
		SPtr<RenderTexture> nativeObject = RenderTextureEx::Create(tmpcolorSurface);
		new (B3DAllocate<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, nativeObject);
	}

	void ScriptRenderTexture::InternalCreate1(MonoObject* managedInstance, MonoObject* colorSurface, MonoObject* depthStencilSurface)
	{
		TResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptObjectWrappercolorSurface;
		scriptObjectWrappercolorSurface = ScriptTexture::ToNative(colorSurface);
		if(scriptObjectWrappercolorSurface != nullptr)
			tmpcolorSurface = scriptObjectWrappercolorSurface->GetHandle();
		TResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptObjectWrapperdepthStencilSurface;
		scriptObjectWrapperdepthStencilSurface = ScriptTexture::ToNative(depthStencilSurface);
		if(scriptObjectWrapperdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = scriptObjectWrapperdepthStencilSurface->GetHandle();
		SPtr<RenderTexture> nativeObject = RenderTextureEx::Create(tmpcolorSurface, tmpdepthStencilSurface);
		new (B3DAllocate<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, nativeObject);
	}

	void ScriptRenderTexture::InternalCreate2(MonoObject* managedInstance, MonoArray* colorSurface)
	{
		Vector<TResourceHandle<Texture>> nativeArraycolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray scriptArraycolorSurface(colorSurface);
			nativeArraycolorSurface.resize(scriptArraycolorSurface.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycolorSurface.Size(); elementIndex++)
			{
				ScriptTexture* scriptObjectWrappercolorSurface;
				scriptObjectWrappercolorSurface = ScriptTexture::ToNative(scriptArraycolorSurface.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercolorSurface != nullptr)
				{
					TResourceHandle<Texture> arrayElementPointercolorSurface = scriptObjectWrappercolorSurface->GetHandle();
					nativeArraycolorSurface[elementIndex] = arrayElementPointercolorSurface;
				}
			}
		}
		SPtr<RenderTexture> nativeObject = RenderTextureEx::Create(nativeArraycolorSurface);
		new (B3DAllocate<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, nativeObject);
	}

	void ScriptRenderTexture::InternalCreate3(MonoObject* managedInstance, MonoArray* colorSurface, MonoObject* depthStencilSurface)
	{
		Vector<TResourceHandle<Texture>> nativeArraycolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray scriptArraycolorSurface(colorSurface);
			nativeArraycolorSurface.resize(scriptArraycolorSurface.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycolorSurface.Size(); elementIndex++)
			{
				ScriptTexture* scriptObjectWrappercolorSurface;
				scriptObjectWrappercolorSurface = ScriptTexture::ToNative(scriptArraycolorSurface.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercolorSurface != nullptr)
				{
					TResourceHandle<Texture> arrayElementPointercolorSurface = scriptObjectWrappercolorSurface->GetHandle();
					nativeArraycolorSurface[elementIndex] = arrayElementPointercolorSurface;
				}
			}

		}
		TResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptObjectWrapperdepthStencilSurface;
		scriptObjectWrapperdepthStencilSurface = ScriptTexture::ToNative(depthStencilSurface);
		if(scriptObjectWrapperdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = scriptObjectWrapperdepthStencilSurface->GetHandle();
		SPtr<RenderTexture> nativeObject = RenderTextureEx::Create(nativeArraycolorSurface, tmpdepthStencilSurface);
		new (B3DAllocate<ScriptRenderTexture>())ScriptRenderTexture(managedInstance, nativeObject);
	}

	MonoObject* ScriptRenderTexture::InternalGetColorSurface(ScriptRenderTexture* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = RenderTextureEx::GetColorSurface(self->GetInternal());

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoArray* ScriptRenderTexture::InternalGetColorSurfaces(ScriptRenderTexture* self)
	{
		Vector<TResourceHandle<Texture>> nativeArray__output;
		nativeArray__output = RenderTextureEx::GetColorSurfaces(self->GetInternal());

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptTexture>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptResourceBase* scriptObjectWrapper__output;
			scriptObjectWrapper__output = ScriptResourceManager::Instance().GetScriptResource(nativeArray__output[elementIndex], true);
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetManagedInstance());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptRenderTexture::InternalGetDepthStencilSurface(ScriptRenderTexture* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = RenderTextureEx::GetDepthStencilSurface(self->GetInternal());

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}
}
