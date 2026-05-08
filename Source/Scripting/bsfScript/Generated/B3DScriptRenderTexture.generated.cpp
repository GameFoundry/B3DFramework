//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRenderTexture.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptTexture.generated.h"
#include "B3DScriptRenderTexture.generated.h"
#include "../Extensions/B3DRenderTargetEx.h"

namespace b3d
{
	ScriptRenderTexture::ScriptRenderTexture(const TShared<RenderTexture>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRenderTexture::~ScriptRenderTexture()
	{
		UnregisterEvents();
	}

	void ScriptRenderTexture::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptRenderTexture::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptRenderTexture::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptRenderTexture::InternalCreate1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptRenderTexture::InternalCreate2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create3", (void*)&ScriptRenderTexture::InternalCreate3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColorSurface", (void*)&ScriptRenderTexture::InternalGetColorSurface);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColorSurfaces", (void*)&ScriptRenderTexture::InternalGetColorSurfaces);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDepthStencilSurface", (void*)&ScriptRenderTexture::InternalGetDepthStencilSurface);

	}

	MonoObject* ScriptRenderTexture::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptRenderTexture::InternalCreate(MonoObject* scriptObject, PixelFormat format, int32_t width, int32_t height, int32_t numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
	{
		TShared<RenderTexture> nativeObject = RenderTextureEx::Create(format, width, height, numSamples, gammaCorrection, createDepth, depthStencilFormat);
		ScriptObjectWrapper::Create<ScriptRenderTexture>(nativeObject, scriptObject);
	}

	void ScriptRenderTexture::InternalCreate0(MonoObject* scriptObject, MonoObject* colorSurface)
	{
		TResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptObjectWrappercolorSurface;
		scriptObjectWrappercolorSurface = ScriptTexture::GetScriptObjectWrapper(colorSurface);
		if(scriptObjectWrappercolorSurface != nullptr)
			tmpcolorSurface = B3DStaticResourceCast<Texture>(scriptObjectWrappercolorSurface->GetBaseNativeObjectAsHandle());
		TShared<RenderTexture> nativeObject = RenderTextureEx::Create(tmpcolorSurface);
		ScriptObjectWrapper::Create<ScriptRenderTexture>(nativeObject, scriptObject);
	}

	void ScriptRenderTexture::InternalCreate1(MonoObject* scriptObject, MonoObject* colorSurface, MonoObject* depthStencilSurface)
	{
		TResourceHandle<Texture> tmpcolorSurface;
		ScriptTexture* scriptObjectWrappercolorSurface;
		scriptObjectWrappercolorSurface = ScriptTexture::GetScriptObjectWrapper(colorSurface);
		if(scriptObjectWrappercolorSurface != nullptr)
			tmpcolorSurface = B3DStaticResourceCast<Texture>(scriptObjectWrappercolorSurface->GetBaseNativeObjectAsHandle());
		TResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptObjectWrapperdepthStencilSurface;
		scriptObjectWrapperdepthStencilSurface = ScriptTexture::GetScriptObjectWrapper(depthStencilSurface);
		if(scriptObjectWrapperdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = B3DStaticResourceCast<Texture>(scriptObjectWrapperdepthStencilSurface->GetBaseNativeObjectAsHandle());
		TShared<RenderTexture> nativeObject = RenderTextureEx::Create(tmpcolorSurface, tmpdepthStencilSurface);
		ScriptObjectWrapper::Create<ScriptRenderTexture>(nativeObject, scriptObject);
	}

	void ScriptRenderTexture::InternalCreate2(MonoObject* scriptObject, MonoArray* colorSurface)
	{
		Vector<TResourceHandle<Texture>> nativeArraycolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray scriptArraycolorSurface(colorSurface);
			nativeArraycolorSurface.resize(scriptArraycolorSurface.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycolorSurface.Size(); elementIndex++)
			{
				TResourceHandle<Texture> arrayElementPointercolorSurface;
				ScriptTexture* scriptObjectWrappercolorSurface;
				scriptObjectWrappercolorSurface = ScriptTexture::GetScriptObjectWrapper(scriptArraycolorSurface.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercolorSurface != nullptr)
				{
					arrayElementPointercolorSurface = B3DStaticResourceCast<Texture>(scriptObjectWrappercolorSurface->GetBaseNativeObjectAsHandle());
					nativeArraycolorSurface[elementIndex] = arrayElementPointercolorSurface;
				}
			}
		}
		TShared<RenderTexture> nativeObject = RenderTextureEx::Create(nativeArraycolorSurface);
		ScriptObjectWrapper::Create<ScriptRenderTexture>(nativeObject, scriptObject);
	}

	void ScriptRenderTexture::InternalCreate3(MonoObject* scriptObject, MonoArray* colorSurface, MonoObject* depthStencilSurface)
	{
		Vector<TResourceHandle<Texture>> nativeArraycolorSurface;
		if(colorSurface != nullptr)
		{
			ScriptArray scriptArraycolorSurface(colorSurface);
			nativeArraycolorSurface.resize(scriptArraycolorSurface.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycolorSurface.Size(); elementIndex++)
			{
				TResourceHandle<Texture> arrayElementPointercolorSurface;
				ScriptTexture* scriptObjectWrappercolorSurface;
				scriptObjectWrappercolorSurface = ScriptTexture::GetScriptObjectWrapper(scriptArraycolorSurface.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercolorSurface != nullptr)
				{
					arrayElementPointercolorSurface = B3DStaticResourceCast<Texture>(scriptObjectWrappercolorSurface->GetBaseNativeObjectAsHandle());
					nativeArraycolorSurface[elementIndex] = arrayElementPointercolorSurface;
				}
			}

		}
		TResourceHandle<Texture> tmpdepthStencilSurface;
		ScriptTexture* scriptObjectWrapperdepthStencilSurface;
		scriptObjectWrapperdepthStencilSurface = ScriptTexture::GetScriptObjectWrapper(depthStencilSurface);
		if(scriptObjectWrapperdepthStencilSurface != nullptr)
			tmpdepthStencilSurface = B3DStaticResourceCast<Texture>(scriptObjectWrapperdepthStencilSurface->GetBaseNativeObjectAsHandle());
		TShared<RenderTexture> nativeObject = RenderTextureEx::Create(nativeArraycolorSurface, tmpdepthStencilSurface);
		ScriptObjectWrapper::Create<ScriptRenderTexture>(nativeObject, scriptObject);
	}

	MonoObject* ScriptRenderTexture::InternalGetColorSurface(ScriptRenderTexture* self)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = RenderTextureEx::GetColorSurface(std::static_pointer_cast<RenderTexture>(self->GetBaseNativeObjectAsShared()));

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoArray* ScriptRenderTexture::InternalGetColorSurfaces(ScriptRenderTexture* self)
	{
		Vector<TResourceHandle<Texture>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = RenderTextureEx::GetColorSurfaces(std::static_pointer_cast<RenderTexture>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptTexture>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			MonoObject* tempscriptArray__output = nullptr;
			if(nativeArray__output[elementIndex])
			tempscriptArray__output = ScriptResourceWrapper::GetOrCreateScriptObject(nativeArray__output[elementIndex]);
			scriptArray__output.Set(elementIndex, tempscriptArray__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptRenderTexture::InternalGetDepthStencilSurface(ScriptRenderTexture* self)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = RenderTextureEx::GetDepthStencilSurface(std::static_pointer_cast<RenderTexture>(self->GetBaseNativeObjectAsShared()));

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}
}
