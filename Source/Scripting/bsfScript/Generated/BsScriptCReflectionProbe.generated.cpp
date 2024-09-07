//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCReflectionProbe.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCReflectionProbe.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	ScriptReflectionProbe::ScriptReflectionProbe(const GameObjectHandle<CReflectionProbe>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptReflectionProbe::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptReflectionProbe::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptReflectionProbe::InternalSetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptReflectionProbe::InternalGetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptReflectionProbe::InternalSetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptReflectionProbe::InternalGetExtents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptReflectionProbe::InternalSetExtents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCustomTexture", (void*)&ScriptReflectionProbe::InternalGetCustomTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCustomTexture", (void*)&ScriptReflectionProbe::InternalSetCustomTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Capture", (void*)&ScriptReflectionProbe::InternalCapture);

	}

	MonoObject* ScriptReflectionProbe::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	ReflectionProbeType ScriptReflectionProbe::InternalGetType(ScriptReflectionProbe* self)
	{
		ReflectionProbeType tmp__output;
		tmp__output = static_cast<CReflectionProbe*>(self->GetNativeObject())->GetType();

		ReflectionProbeType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetType(ScriptReflectionProbe* self, ReflectionProbeType type)
	{
		static_cast<CReflectionProbe*>(self->GetNativeObject())->SetType(type);
	}

	float ScriptReflectionProbe::InternalGetRadius(ScriptReflectionProbe* self)
	{
		float tmp__output;
		tmp__output = static_cast<CReflectionProbe*>(self->GetNativeObject())->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetRadius(ScriptReflectionProbe* self, float radius)
	{
		static_cast<CReflectionProbe*>(self->GetNativeObject())->SetRadius(radius);
	}

	void ScriptReflectionProbe::InternalGetExtents(ScriptReflectionProbe* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CReflectionProbe*>(self->GetNativeObject())->GetExtents();

		*__output = tmp__output;
	}

	void ScriptReflectionProbe::InternalSetExtents(ScriptReflectionProbe* self, TVector3<float>* extents)
	{
		static_cast<CReflectionProbe*>(self->GetNativeObject())->SetExtents(*extents);
	}

	MonoObject* ScriptReflectionProbe::InternalGetCustomTexture(ScriptReflectionProbe* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = static_cast<CReflectionProbe*>(self->GetNativeObject())->GetCustomTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetCustomTexture(ScriptReflectionProbe* self, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::GetScriptObjectWrapper(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetNativeObject());
		static_cast<CReflectionProbe*>(self->GetNativeObject())->SetCustomTexture(tmptexture);
	}

	void ScriptReflectionProbe::InternalCapture(ScriptReflectionProbe* self)
	{
		static_cast<CReflectionProbe*>(self->GetNativeObject())->Capture();
	}
}
