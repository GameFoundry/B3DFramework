//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTexture.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Image/B3DTexture.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "Wrappers/B3DScriptAsyncOp.h"
#include "B3DScriptPixelData.generated.h"
#include "../../../Foundation/Core/Image/B3DTexture.h"
#include "B3DScriptColor.generated.h"
#include "../Extensions/B3DTextureEx.h"

namespace b3d
{
	ScriptTexture::ScriptTexture(const TResourceHandle<Texture>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptTexture::~ScriptTexture()
	{
		UnregisterEvents();
	}

	void ScriptTexture::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptTexture::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ReadData", (void*)&ScriptTexture::InternalReadData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptTexture::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPixelFormat", (void*)&ScriptTexture::InternalGetPixelFormat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUsage", (void*)&ScriptTexture::InternalGetUsage);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptTexture::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWidth", (void*)&ScriptTexture::InternalGetWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptTexture::InternalGetHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDepth", (void*)&ScriptTexture::InternalGetDepth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGammaCorrection", (void*)&ScriptTexture::InternalGetGammaCorrection);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSampleCount", (void*)&ScriptTexture::InternalGetSampleCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMipmapCount", (void*)&ScriptTexture::InternalGetMipmapCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPixels", (void*)&ScriptTexture::InternalGetPixels);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPixels", (void*)&ScriptTexture::InternalSetPixels);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPixelsArray", (void*)&ScriptTexture::InternalSetPixelsArray);

	}

	MonoObject* ScriptTexture::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptTexture::InternalGetRef(ScriptTexture* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoObject* ScriptTexture::InternalReadData(ScriptTexture* self, uint32_t face, uint32_t mipLevel)
	{
		TAsyncOp<SPtr<PixelData>> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Texture*>(self->GetNativeObject())->ReadData(face, mipLevel);

		MonoObject* __output;
		auto fnConvertCallback = [](const Any& returnValue)
		{
			SPtr<PixelData> nativeObject = AnyCast<SPtr<PixelData>>(returnValue);
			MonoObject* scriptObject;
			scriptObject = ScriptPixelData::GetOrCreateScriptObject(nativeObject);
			return scriptObject;
		};

;		__output = ScriptAsyncOpBase::Create(tmp__output, fnConvertCallback, ScriptPixelData::GetMetaData()->ScriptClass);

		return __output;
	}

	void ScriptTexture::InternalCreate(MonoObject* scriptObject, PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, TextureType texType, int32_t usage, uint32_t numSamples, bool hasMipmaps, bool gammaCorrection)
	{
		TResourceHandle<Texture> nativeObject = TextureEx::Create(format, width, height, depth, texType, usage, numSamples, hasMipmaps, gammaCorrection);
		ScriptObjectWrapper::Create<ScriptTexture>(nativeObject, scriptObject);
	}

	PixelFormat ScriptTexture::InternalGetPixelFormat(ScriptTexture* self)
	{
		PixelFormat tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetPixelFormat(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptTexture::InternalGetUsage(ScriptTexture* self)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetUsage(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	TextureType ScriptTexture::InternalGetType(ScriptTexture* self)
	{
		TextureType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetType(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		TextureType __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetWidth(ScriptTexture* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetWidth(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetHeight(ScriptTexture* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetHeight(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetDepth(ScriptTexture* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetDepth(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptTexture::InternalGetGammaCorrection(ScriptTexture* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetGammaCorrection(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetSampleCount(ScriptTexture* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetSampleCount(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetMipmapCount(ScriptTexture* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetMipmapCount(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptTexture::InternalGetPixels(ScriptTexture* self, uint32_t face, uint32_t mipLevel)
	{
		SPtr<PixelData> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = TextureEx::GetPixels(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()), face, mipLevel);

		MonoObject* __output;
		__output = ScriptPixelData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptTexture::InternalSetPixels(ScriptTexture* self, MonoObject* data, uint32_t face, uint32_t mipLevel)
	{
		if(!self->IsNativeObjectValid())
			return;

		SPtr<PixelData> tmpdata;
		ScriptPixelData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptPixelData::GetScriptObjectWrapper(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = std::static_pointer_cast<PixelData>(scriptObjectWrapperdata->GetBaseNativeObjectAsShared());
		TextureEx::SetPixels(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()), tmpdata, face, mipLevel);
	}

	void ScriptTexture::InternalSetPixelsArray(ScriptTexture* self, MonoArray* colors, uint32_t face, uint32_t mipLevel)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<Color> nativeArraycolors;
		if(colors != nullptr)
		{
			ScriptArray scriptArraycolors(colors);
			nativeArraycolors.resize(scriptArraycolors.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycolors.Size(); elementIndex++)
			{
				nativeArraycolors[elementIndex] = scriptArraycolors.Get<Color>(elementIndex);
			}

		}
		TextureEx::SetPixelsArray(B3DStaticResourceCast<Texture>(self->GetBaseNativeObjectAsHandle()), nativeArraycolors, face, mipLevel);
	}
}
