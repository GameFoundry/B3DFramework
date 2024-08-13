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
	ScriptTextureImportOptions::ScriptTextureImportOptions(MonoObject* managedInstance, const SPtr<TextureImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptTextureImportOptions::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptTextureImportOptions::InternalGetFormat);
		metaData.ScriptClass->AddInternalCall("Internal_SetFormat", (void*)&ScriptTextureImportOptions::InternalSetFormat);
		metaData.ScriptClass->AddInternalCall("Internal_GetGenerateMips", (void*)&ScriptTextureImportOptions::InternalGetGenerateMips);
		metaData.ScriptClass->AddInternalCall("Internal_SetGenerateMips", (void*)&ScriptTextureImportOptions::InternalSetGenerateMips);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxMip", (void*)&ScriptTextureImportOptions::InternalGetMaxMip);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxMip", (void*)&ScriptTextureImportOptions::InternalSetMaxMip);
		metaData.ScriptClass->AddInternalCall("Internal_GetCpuCached", (void*)&ScriptTextureImportOptions::InternalGetCpuCached);
		metaData.ScriptClass->AddInternalCall("Internal_SetCpuCached", (void*)&ScriptTextureImportOptions::InternalSetCpuCached);
		metaData.ScriptClass->AddInternalCall("Internal_GetSRgb", (void*)&ScriptTextureImportOptions::InternalGetSRgb);
		metaData.ScriptClass->AddInternalCall("Internal_SetSRgb", (void*)&ScriptTextureImportOptions::InternalSetSRgb);
		metaData.ScriptClass->AddInternalCall("Internal_GetCubemap", (void*)&ScriptTextureImportOptions::InternalGetCubemap);
		metaData.ScriptClass->AddInternalCall("Internal_SetCubemap", (void*)&ScriptTextureImportOptions::InternalSetCubemap);
		metaData.ScriptClass->AddInternalCall("Internal_GetCubemapSourceType", (void*)&ScriptTextureImportOptions::InternalGetCubemapSourceType);
		metaData.ScriptClass->AddInternalCall("Internal_SetCubemapSourceType", (void*)&ScriptTextureImportOptions::InternalSetCubemapSourceType);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptTextureImportOptions::InternalCreate);

	}

	MonoObject* ScriptTextureImportOptions::Create(const SPtr<TextureImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTextureImportOptions>()) ScriptTextureImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptTextureImportOptions::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<TextureImportOptions> nativeObject = TextureImportOptions::Create();
		new (B3DAllocate<ScriptTextureImportOptions>())ScriptTextureImportOptions(managedInstance, nativeObject);
	}
	PixelFormat ScriptTextureImportOptions::InternalGetFormat(ScriptTextureImportOptions* self)
	{
		PixelFormat tmp__output;
		tmp__output = self->GetInternal()->Format;

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetFormat(ScriptTextureImportOptions* self, PixelFormat value)
	{
		self->GetInternal()->Format = value;
	}

	bool ScriptTextureImportOptions::InternalGetGenerateMips(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->GenerateMips;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetGenerateMips(ScriptTextureImportOptions* self, bool value)
	{
		self->GetInternal()->GenerateMips = value;
	}

	uint32_t ScriptTextureImportOptions::InternalGetMaxMip(ScriptTextureImportOptions* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->MaxMip;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetMaxMip(ScriptTextureImportOptions* self, uint32_t value)
	{
		self->GetInternal()->MaxMip = value;
	}

	bool ScriptTextureImportOptions::InternalGetCpuCached(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->CpuCached;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetCpuCached(ScriptTextureImportOptions* self, bool value)
	{
		self->GetInternal()->CpuCached = value;
	}

	bool ScriptTextureImportOptions::InternalGetSRgb(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->SRgb;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetSRgb(ScriptTextureImportOptions* self, bool value)
	{
		self->GetInternal()->SRgb = value;
	}

	bool ScriptTextureImportOptions::InternalGetCubemap(ScriptTextureImportOptions* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Cubemap;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetCubemap(ScriptTextureImportOptions* self, bool value)
	{
		self->GetInternal()->Cubemap = value;
	}

	CubemapSourceType ScriptTextureImportOptions::InternalGetCubemapSourceType(ScriptTextureImportOptions* self)
	{
		CubemapSourceType tmp__output;
		tmp__output = self->GetInternal()->CubemapSourceType;

		CubemapSourceType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::InternalSetCubemapSourceType(ScriptTextureImportOptions* self, CubemapSourceType value)
	{
		self->GetInternal()->CubemapSourceType = value;
	}
#endif
}
