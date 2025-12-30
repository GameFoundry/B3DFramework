//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptReflectionProbe.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DReflectionProbe.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Image/B3DTexture.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptReflectionProbe::ScriptReflectionProbe(const TGameObjectHandle<ReflectionProbe>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptReflectionProbe::~ScriptReflectionProbe()
	{
		UnregisterEvents();
	}

	void ScriptReflectionProbe::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCustomTexture", (void*)&ScriptReflectionProbe::InternalSetCustomTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCustomTexture", (void*)&ScriptReflectionProbe::InternalGetCustomTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptReflectionProbe::InternalGetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptReflectionProbe::InternalGetExtents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Capture", (void*)&ScriptReflectionProbe::InternalCapture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptReflectionProbe::InternalSetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptReflectionProbe::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptReflectionProbe::InternalSetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptReflectionProbe::InternalSetExtents);

	}

	MonoObject* ScriptReflectionProbe::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptReflectionProbe::InternalSetCustomTexture(ScriptReflectionProbe* self, MonoObject* texture)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::GetScriptObjectWrapper(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetNativeObject());
		static_cast<ReflectionProbe*>(self->GetNativeObject())->SetCustomTexture(tmptexture);
	}

	MonoObject* ScriptReflectionProbe::InternalGetCustomTexture(ScriptReflectionProbe* self)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ReflectionProbe*>(self->GetNativeObject())->GetCustomTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	float ScriptReflectionProbe::InternalGetRadius(ScriptReflectionProbe* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ReflectionProbe*>(self->GetNativeObject())->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalGetExtents(ScriptReflectionProbe* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<ReflectionProbe*>(self->GetNativeObject())->GetExtents();

		*__output = tmp__output;
	}

	void ScriptReflectionProbe::InternalCapture(ScriptReflectionProbe* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ReflectionProbe*>(self->GetNativeObject())->Capture();
	}

	void ScriptReflectionProbe::InternalSetType(ScriptReflectionProbe* self, ReflectionProbeType type)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ReflectionProbe*>(self->GetNativeObject())->SetType(type);
	}

	ReflectionProbeType ScriptReflectionProbe::InternalGetType(ScriptReflectionProbe* self)
	{
		ReflectionProbeType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ReflectionProbe*>(self->GetNativeObject())->GetType();

		ReflectionProbeType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetRadius(ScriptReflectionProbe* self, float radius)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ReflectionProbe*>(self->GetNativeObject())->SetRadius(radius);
	}

	void ScriptReflectionProbe::InternalSetExtents(ScriptReflectionProbe* self, TVector3<float>* extents)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ReflectionProbe*>(self->GetNativeObject())->SetExtents(*extents);
	}
}
