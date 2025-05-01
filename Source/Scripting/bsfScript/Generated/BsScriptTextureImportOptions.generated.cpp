//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTextureImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptTextureImportOptions.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptTextureImportOptions::ScriptTextureImportOptions(const SPtr<TextureImportOptions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptTextureImportOptions::~ScriptTextureImportOptions()
	{
		UnregisterEvents();
	}

	void ScriptTextureImportOptions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptTextureImportOptions::InternalGetFormat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFormat", (void*)&ScriptTextureImportOptions::InternalSetFormat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGenerateMips", (void*)&ScriptTextureImportOptions::InternalGetGenerateMips);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGenerateMips", (void*)&ScriptTextureImportOptions::InternalSetGenerateMips);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxMip", (void*)&ScriptTextureImportOptions::InternalGetMaxMip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxMip", (void*)&ScriptTextureImportOptions::InternalSetMaxMip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCpuCached", (void*)&ScriptTextureImportOptions::InternalGetCpuCached);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCpuCached", (void*)&ScriptTextureImportOptions::InternalSetCpuCached);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSRgb", (void*)&ScriptTextureImportOptions::InternalGetSRgb);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSRgb", (void*)&ScriptTextureImportOptions::InternalSetSRgb);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCubemap", (void*)&ScriptTextureImportOptions::InternalGetCubemap);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCubemap", (void*)&ScriptTextureImportOptions::InternalSetCubemap);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCubemapSourceType", (void*)&ScriptTextureImportOptions::InternalGetCubemapSourceType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCubemapSourceType", (void*)&ScriptTextureImportOptions::InternalSetCubemapSourceType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptTextureImportOptions::InternalCreate);

	}

	MonoObject* ScriptTextureImportOptions::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptTextureImportOptions::InternalCreate(MonoObject* scriptObject)
	{
		SPtr<TextureImportOptions> nativeObject = TextureImportOptions::Create();
		ScriptObjectWrapper::Create<ScriptTextureImportOptions>(nativeObject, scriptObject);
	}
	PixelFormat ScriptTextureImportOptions::InternalGetFormat(ScriptTextureImportOptions* self)
	{
		PixelFormat tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TextureImportOptions*>(self->GetNativeObject())->Format;

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetFormat(ScriptTextureImportOptions* self, PixelFormat value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TextureImportOptions*>(self->GetNativeObject())->Format = value;
	}

	bool ScriptTextureImportOptions::InternalGetGenerateMips(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TextureImportOptions*>(self->GetNativeObject())->GenerateMips;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetGenerateMips(ScriptTextureImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TextureImportOptions*>(self->GetNativeObject())->GenerateMips = value;
	}

	uint32_t ScriptTextureImportOptions::InternalGetMaxMip(ScriptTextureImportOptions* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TextureImportOptions*>(self->GetNativeObject())->MaxMip;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetMaxMip(ScriptTextureImportOptions* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TextureImportOptions*>(self->GetNativeObject())->MaxMip = value;
	}

	bool ScriptTextureImportOptions::InternalGetCpuCached(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TextureImportOptions*>(self->GetNativeObject())->CpuCached;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetCpuCached(ScriptTextureImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TextureImportOptions*>(self->GetNativeObject())->CpuCached = value;
	}

	bool ScriptTextureImportOptions::InternalGetSRgb(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TextureImportOptions*>(self->GetNativeObject())->SRgb;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetSRgb(ScriptTextureImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TextureImportOptions*>(self->GetNativeObject())->SRgb = value;
	}

	bool ScriptTextureImportOptions::InternalGetCubemap(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TextureImportOptions*>(self->GetNativeObject())->Cubemap;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetCubemap(ScriptTextureImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TextureImportOptions*>(self->GetNativeObject())->Cubemap = value;
	}

	CubemapSourceType ScriptTextureImportOptions::InternalGetCubemapSourceType(ScriptTextureImportOptions* self)
	{
		CubemapSourceType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TextureImportOptions*>(self->GetNativeObject())->CubemapSourceType;

		CubemapSourceType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetCubemapSourceType(ScriptTextureImportOptions* self, CubemapSourceType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TextureImportOptions*>(self->GetNativeObject())->CubemapSourceType = value;
	}
#endif
}
