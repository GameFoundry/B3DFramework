//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptShadowSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptShadowSettings::ScriptShadowSettings(const TShared<ShadowSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptShadowSettings::~ScriptShadowSettings()
	{
		UnregisterEvents();
	}

	void ScriptShadowSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ShadowSettings", (void*)&ScriptShadowSettings::InternalShadowSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDirectionalShadowDistance", (void*)&ScriptShadowSettings::InternalGetDirectionalShadowDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDirectionalShadowDistance", (void*)&ScriptShadowSettings::InternalSetDirectionalShadowDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNumCascades", (void*)&ScriptShadowSettings::InternalGetNumCascades);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNumCascades", (void*)&ScriptShadowSettings::InternalSetNumCascades);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCascadeDistributionExponent", (void*)&ScriptShadowSettings::InternalGetCascadeDistributionExponent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCascadeDistributionExponent", (void*)&ScriptShadowSettings::InternalSetCascadeDistributionExponent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShadowFilteringQuality", (void*)&ScriptShadowSettings::InternalGetShadowFilteringQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShadowFilteringQuality", (void*)&ScriptShadowSettings::InternalSetShadowFilteringQuality);

	}

	MonoObject* ScriptShadowSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptShadowSettings::InternalShadowSettings(MonoObject* scriptObject)
	{
		TShared<ShadowSettings> nativeObject = B3DMakeShared<ShadowSettings>();
		ScriptObjectWrapper::Create<ScriptShadowSettings>(nativeObject, scriptObject);
	}

	float ScriptShadowSettings::InternalGetDirectionalShadowDistance(ScriptShadowSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ShadowSettings*>(self->GetNativeObject())->DirectionalShadowDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetDirectionalShadowDistance(ScriptShadowSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ShadowSettings*>(self->GetNativeObject())->DirectionalShadowDistance = value;
	}

	uint32_t ScriptShadowSettings::InternalGetNumCascades(ScriptShadowSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ShadowSettings*>(self->GetNativeObject())->NumCascades;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetNumCascades(ScriptShadowSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ShadowSettings*>(self->GetNativeObject())->NumCascades = value;
	}

	float ScriptShadowSettings::InternalGetCascadeDistributionExponent(ScriptShadowSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ShadowSettings*>(self->GetNativeObject())->CascadeDistributionExponent;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetCascadeDistributionExponent(ScriptShadowSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ShadowSettings*>(self->GetNativeObject())->CascadeDistributionExponent = value;
	}

	uint32_t ScriptShadowSettings::InternalGetShadowFilteringQuality(ScriptShadowSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ShadowSettings*>(self->GetNativeObject())->ShadowFilteringQuality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetShadowFilteringQuality(ScriptShadowSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ShadowSettings*>(self->GetNativeObject())->ShadowFilteringQuality = value;
	}
}
