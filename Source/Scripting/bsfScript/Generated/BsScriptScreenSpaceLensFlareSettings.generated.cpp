//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScreenSpaceLensFlareSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

using namespace bs;
ScriptScreenSpaceLensFlareSettings::ScriptScreenSpaceLensFlareSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceLensFlareSettings>& value)
	: TScriptReflectable(managedInstance, value)
{
}

void ScriptScreenSpaceLensFlareSettings::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_ScreenSpaceLensFlareSettings", (void*)&ScriptScreenSpaceLensFlareSettings::InternalScreenSpaceLensFlareSettings);
	metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetEnabled);
	metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetEnabled);
	metaData.ScriptClass->AddInternalCall("Internal_GetDownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetDownsampleCount);
	metaData.ScriptClass->AddInternalCall("Internal_SetDownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetDownsampleCount);
	metaData.ScriptClass->AddInternalCall("Internal_GetThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetThreshold);
	metaData.ScriptClass->AddInternalCall("Internal_SetThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetThreshold);
	metaData.ScriptClass->AddInternalCall("Internal_GetGhostCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetGhostCount);
	metaData.ScriptClass->AddInternalCall("Internal_SetGhostCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetGhostCount);
	metaData.ScriptClass->AddInternalCall("Internal_GetGhostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetGhostSpacing);
	metaData.ScriptClass->AddInternalCall("Internal_SetGhostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetGhostSpacing);
	metaData.ScriptClass->AddInternalCall("Internal_GetBrightness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetBrightness);
	metaData.ScriptClass->AddInternalCall("Internal_SetBrightness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetBrightness);
	metaData.ScriptClass->AddInternalCall("Internal_GetFilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetFilterSize);
	metaData.ScriptClass->AddInternalCall("Internal_SetFilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetFilterSize);
	metaData.ScriptClass->AddInternalCall("Internal_GetHalo", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHalo);
	metaData.ScriptClass->AddInternalCall("Internal_SetHalo", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHalo);
	metaData.ScriptClass->AddInternalCall("Internal_GetHaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloRadius);
	metaData.ScriptClass->AddInternalCall("Internal_SetHaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloRadius);
	metaData.ScriptClass->AddInternalCall("Internal_GetHaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloThickness);
	metaData.ScriptClass->AddInternalCall("Internal_SetHaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloThickness);
	metaData.ScriptClass->AddInternalCall("Internal_GetHaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloThreshold);
	metaData.ScriptClass->AddInternalCall("Internal_SetHaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloThreshold);
	metaData.ScriptClass->AddInternalCall("Internal_GetHaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloAspectRatio);
	metaData.ScriptClass->AddInternalCall("Internal_SetHaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloAspectRatio);
	metaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberration);
	metaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberration);
	metaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberrationOffset);
	metaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberrationOffset);
	metaData.ScriptClass->AddInternalCall("Internal_GetBicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetBicubicUpsampling);
	metaData.ScriptClass->AddInternalCall("Internal_SetBicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetBicubicUpsampling);
}

MonoObject* ScriptScreenSpaceLensFlareSettings::Create(const SPtr<ScreenSpaceLensFlareSettings>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptScreenSpaceLensFlareSettings>()) ScriptScreenSpaceLensFlareSettings(managedInstance, value);
	return managedInstance;
}

void ScriptScreenSpaceLensFlareSettings::InternalScreenSpaceLensFlareSettings(MonoObject* managedInstance)
{
	SPtr<ScreenSpaceLensFlareSettings> instance = B3DMakeShared<ScreenSpaceLensFlareSettings>();
	new(B3DAllocate<ScriptScreenSpaceLensFlareSettings>()) ScriptScreenSpaceLensFlareSettings(managedInstance, instance);
}

bool ScriptScreenSpaceLensFlareSettings::InternalGetEnabled(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->Enabled;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetEnabled(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
{
	thisPtr->GetInternal()->Enabled = value;
}

uint32_t ScriptScreenSpaceLensFlareSettings::InternalGetDownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->DownsampleCount;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetDownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->DownsampleCount = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->Threshold;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->Threshold = value;
}

uint32_t ScriptScreenSpaceLensFlareSettings::InternalGetGhostCount(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->GhostCount;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetGhostCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->GhostCount = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetGhostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->GhostSpacing;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetGhostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->GhostSpacing = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetBrightness(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->Brightness;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetBrightness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->Brightness = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetFilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->FilterSize;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetFilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->FilterSize = value;
}

bool ScriptScreenSpaceLensFlareSettings::InternalGetHalo(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->Halo;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetHalo(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
{
	thisPtr->GetInternal()->Halo = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetHaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->HaloRadius;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetHaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->HaloRadius = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetHaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->HaloThickness;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetHaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->HaloThickness = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetHaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->HaloThreshold;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetHaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->HaloThreshold = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->HaloAspectRatio;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->HaloAspectRatio = value;
}

bool ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->ChromaticAberration;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
{
	thisPtr->GetInternal()->ChromaticAberration = value;
}

float ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->ChromaticAberrationOffset;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->ChromaticAberrationOffset = value;
}

bool ScriptScreenSpaceLensFlareSettings::InternalGetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->BicubicUpsampling;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptScreenSpaceLensFlareSettings::InternalSetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
{
	thisPtr->GetInternal()->BicubicUpsampling = value;
}
