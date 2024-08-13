//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	ScriptTexture::ScriptTexture(MonoObject* managedInstance, const TResourceHandle<Texture>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptTexture::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptTexture::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_ReadData", (void*)&ScriptTexture::InternalReadData);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptTexture::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_GetPixelFormat", (void*)&ScriptTexture::InternalGetPixelFormat);
		metaData.ScriptClass->AddInternalCall("Internal_GetUsage", (void*)&ScriptTexture::InternalGetUsage);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptTexture::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetWidth", (void*)&ScriptTexture::InternalGetWidth);
		metaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptTexture::InternalGetHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetDepth", (void*)&ScriptTexture::InternalGetDepth);
		metaData.ScriptClass->AddInternalCall("Internal_GetGammaCorrection", (void*)&ScriptTexture::InternalGetGammaCorrection);
		metaData.ScriptClass->AddInternalCall("Internal_GetSampleCount", (void*)&ScriptTexture::InternalGetSampleCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetMipmapCount", (void*)&ScriptTexture::InternalGetMipmapCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetPixels", (void*)&ScriptTexture::InternalGetPixels);
		metaData.ScriptClass->AddInternalCall("Internal_SetPixels", (void*)&ScriptTexture::InternalSetPixels);
		metaData.ScriptClass->AddInternalCall("Internal_SetPixelsArray", (void*)&ScriptTexture::InternalSetPixelsArray);

	}

	 MonoObject*ScriptTexture::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptTexture::InternalGetRef(ScriptTexture* self)
	{
		return self->GetRRef();
	}

	MonoObject* ScriptTexture::InternalReadData(ScriptTexture* self, uint32_t face, uint32_t mipLevel)
	{
		TAsyncOp<SPtr<PixelData>> tmp__output;
		tmp__output = self->GetHandle()->ReadData(face, mipLevel);

		MonoObject* __output;
		auto fnConvertCallback = [](const Any& returnValue)
		{
			SPtr<PixelData> nativeObject = AnyCast<SPtr<PixelData>>(returnValue);
			MonoObject* scriptObject;
			scriptObject = ScriptPixelData::Create(nativeObject);
			return scriptObject;
		};

;		__output = ScriptAsyncOpBase::Create(tmp__output, fnConvertCallback, ScriptPixelData::GetMetaData()->ScriptClass);

		return __output;
	}

	void ScriptTexture::InternalCreate(MonoObject* managedInstance, PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, TextureType texType, TextureUsage usage, uint32_t numSamples, bool hasMipmaps, bool gammaCorrection)
	{
		TResourceHandle<Texture> nativeObject = TextureEx::Create(format, width, height, depth, texType, usage, numSamples, hasMipmaps, gammaCorrection);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}

	PixelFormat ScriptTexture::InternalGetPixelFormat(ScriptTexture* self)
	{
		PixelFormat tmp__output;
		tmp__output = TextureEx::GetPixelFormat(self->GetHandle());

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	TextureUsage ScriptTexture::InternalGetUsage(ScriptTexture* self)
	{
		TextureUsage tmp__output;
		tmp__output = TextureEx::GetUsage(self->GetHandle());

		TextureUsage __output;
		__output = tmp__output;

		return __output;
	}

	TextureType ScriptTexture::InternalGetType(ScriptTexture* self)
	{
		TextureType tmp__output;
		tmp__output = TextureEx::GetType(self->GetHandle());

		TextureType __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetWidth(ScriptTexture* self)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::GetWidth(self->GetHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetHeight(ScriptTexture* self)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::GetHeight(self->GetHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetDepth(ScriptTexture* self)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::GetDepth(self->GetHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptTexture::InternalGetGammaCorrection(ScriptTexture* self)
	{
		bool tmp__output;
		tmp__output = TextureEx::GetGammaCorrection(self->GetHandle());

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetSampleCount(ScriptTexture* self)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::GetSampleCount(self->GetHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptTexture::InternalGetMipmapCount(ScriptTexture* self)
	{
		uint32_t tmp__output;
		tmp__output = TextureEx::GetMipmapCount(self->GetHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptTexture::InternalGetPixels(ScriptTexture* self, uint32_t face, uint32_t mipLevel)
	{
		SPtr<PixelData> tmp__output;
		tmp__output = TextureEx::GetPixels(self->GetHandle(), face, mipLevel);

		MonoObject* __output;
		__output = ScriptPixelData::Create(tmp__output);

		return __output;
	}

	void ScriptTexture::InternalSetPixels(ScriptTexture* self, MonoObject* data, uint32_t face, uint32_t mipLevel)
	{
		SPtr<PixelData> tmpdata;
		ScriptPixelData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptPixelData::ToNative(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = scriptObjectWrapperdata->GetInternal();
		TextureEx::SetPixels(self->GetHandle(), tmpdata, face, mipLevel);
	}

	void ScriptTexture::InternalSetPixelsArray(ScriptTexture* self, MonoArray* colors, uint32_t face, uint32_t mipLevel)
	{
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
		TextureEx::SetPixelsArray(self->GetHandle(), nativeArraycolors, face, mipLevel);
	}
}
