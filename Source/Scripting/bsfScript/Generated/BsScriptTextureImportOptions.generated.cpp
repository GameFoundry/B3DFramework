//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTextureImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptTextureImportOptions.generated.h"

using namespace bs;
#if !BS_IS_BANSHEE3D
ScriptTextureImportOptions::ScriptTextureImportOptions(MonoObject* managedInstance, const SPtr<TextureImportOptions>& value)
	: TScriptReflectable(managedInstance, value)
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
	new(B3DAllocate<ScriptTextureImportOptions>()) ScriptTextureImportOptions(managedInstance, value);
	return managedInstance;
}

void ScriptTextureImportOptions::InternalCreate(MonoObject* managedInstance)
{
	SPtr<TextureImportOptions> instance = TextureImportOptions::Create();
	new(B3DAllocate<ScriptTextureImportOptions>()) ScriptTextureImportOptions(managedInstance, instance);
}

PixelFormat ScriptTextureImportOptions::InternalGetFormat(ScriptTextureImportOptions* thisPtr)
{
	PixelFormat tmp__output;
	tmp__output = thisPtr->GetInternal()->Format;

	PixelFormat __output;
	__output = tmp__output;

	return __output;
}

void ScriptTextureImportOptions::InternalSetFormat(ScriptTextureImportOptions* thisPtr, PixelFormat value)
{
	thisPtr->GetInternal()->Format = value;
}

bool ScriptTextureImportOptions::InternalGetGenerateMips(ScriptTextureImportOptions* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->GenerateMips;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptTextureImportOptions::InternalSetGenerateMips(ScriptTextureImportOptions* thisPtr, bool value)
{
	thisPtr->GetInternal()->GenerateMips = value;
}

uint32_t ScriptTextureImportOptions::InternalGetMaxMip(ScriptTextureImportOptions* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->MaxMip;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptTextureImportOptions::InternalSetMaxMip(ScriptTextureImportOptions* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->MaxMip = value;
}

bool ScriptTextureImportOptions::InternalGetCpuCached(ScriptTextureImportOptions* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->CpuCached;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptTextureImportOptions::InternalSetCpuCached(ScriptTextureImportOptions* thisPtr, bool value)
{
	thisPtr->GetInternal()->CpuCached = value;
}

bool ScriptTextureImportOptions::InternalGetSRgb(ScriptTextureImportOptions* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->SRgb;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptTextureImportOptions::InternalSetSRgb(ScriptTextureImportOptions* thisPtr, bool value)
{
	thisPtr->GetInternal()->SRgb = value;
}

bool ScriptTextureImportOptions::InternalGetCubemap(ScriptTextureImportOptions* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->Cubemap;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptTextureImportOptions::InternalSetCubemap(ScriptTextureImportOptions* thisPtr, bool value)
{
	thisPtr->GetInternal()->Cubemap = value;
}

CubemapSourceType ScriptTextureImportOptions::InternalGetCubemapSourceType(ScriptTextureImportOptions* thisPtr)
{
	CubemapSourceType tmp__output;
	tmp__output = thisPtr->GetInternal()->CubemapSourceType;

	CubemapSourceType __output;
	__output = tmp__output;

	return __output;
}

void ScriptTextureImportOptions::InternalSetCubemapSourceType(ScriptTextureImportOptions* thisPtr, CubemapSourceType value)
{
	thisPtr->GetInternal()->CubemapSourceType = value;
}
#endif
