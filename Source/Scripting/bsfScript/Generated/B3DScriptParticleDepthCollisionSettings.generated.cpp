//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleDepthCollisionSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptParticleDepthCollisionSettings::ScriptParticleDepthCollisionSettings(const TShared<ParticleDepthCollisionSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleDepthCollisionSettings::~ScriptParticleDepthCollisionSettings()
	{
		UnregisterEvents();
	}

	void ScriptParticleDepthCollisionSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ParticleDepthCollisionSettings", (void*)&ScriptParticleDepthCollisionSettings::InternalParticleDepthCollisionSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptParticleDepthCollisionSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptParticleDepthCollisionSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRestitution", (void*)&ScriptParticleDepthCollisionSettings::InternalGetRestitution);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRestitution", (void*)&ScriptParticleDepthCollisionSettings::InternalSetRestitution);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDampening", (void*)&ScriptParticleDepthCollisionSettings::InternalGetDampening);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDampening", (void*)&ScriptParticleDepthCollisionSettings::InternalSetDampening);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRadiusScale", (void*)&ScriptParticleDepthCollisionSettings::InternalGetRadiusScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRadiusScale", (void*)&ScriptParticleDepthCollisionSettings::InternalSetRadiusScale);

	}

	MonoObject* ScriptParticleDepthCollisionSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleDepthCollisionSettings::InternalParticleDepthCollisionSettings(MonoObject* scriptObject)
	{
		TShared<ParticleDepthCollisionSettings> nativeObject = B3DMakeShared<ParticleDepthCollisionSettings>();
		ScriptObjectWrapper::Create<ScriptParticleDepthCollisionSettings>(nativeObject, scriptObject);
	}

	bool ScriptParticleDepthCollisionSettings::InternalGetEnabled(ScriptParticleDepthCollisionSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetEnabled(ScriptParticleDepthCollisionSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->Enabled = value;
	}

	float ScriptParticleDepthCollisionSettings::InternalGetRestitution(ScriptParticleDepthCollisionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->Restitution;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetRestitution(ScriptParticleDepthCollisionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->Restitution = value;
	}

	float ScriptParticleDepthCollisionSettings::InternalGetDampening(ScriptParticleDepthCollisionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->Dampening;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetDampening(ScriptParticleDepthCollisionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->Dampening = value;
	}

	float ScriptParticleDepthCollisionSettings::InternalGetRadiusScale(ScriptParticleDepthCollisionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->RadiusScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetRadiusScale(ScriptParticleDepthCollisionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleDepthCollisionSettings*>(self->GetNativeObject())->RadiusScale = value;
	}
}
