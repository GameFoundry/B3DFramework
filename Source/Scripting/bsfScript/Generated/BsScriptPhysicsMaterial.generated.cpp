//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicsMaterial.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMaterial.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMaterial.h"

namespace bs
{
	ScriptPhysicsMaterial::ScriptPhysicsMaterial(const TResourceHandle<PhysicsMaterial>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptPhysicsMaterial::~ScriptPhysicsMaterial()
	{
		UnregisterEvents();
	}

	void ScriptPhysicsMaterial::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptPhysicsMaterial::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetStaticFriction", (void*)&ScriptPhysicsMaterial::InternalSetStaticFriction);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetStaticFriction", (void*)&ScriptPhysicsMaterial::InternalGetStaticFriction);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDynamicFriction", (void*)&ScriptPhysicsMaterial::InternalSetDynamicFriction);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDynamicFriction", (void*)&ScriptPhysicsMaterial::InternalGetDynamicFriction);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRestitutionCoefficient", (void*)&ScriptPhysicsMaterial::InternalSetRestitutionCoefficient);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRestitutionCoefficient", (void*)&ScriptPhysicsMaterial::InternalGetRestitutionCoefficient);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptPhysicsMaterial::InternalCreate);

	}

	MonoObject* ScriptPhysicsMaterial::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptPhysicsMaterial::InternalGetRef(ScriptPhysicsMaterial* self)
	{
		return self->GetOrCreateResourceReference();
	}

	void ScriptPhysicsMaterial::InternalSetStaticFriction(ScriptPhysicsMaterial* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PhysicsMaterial*>(self->GetNativeObject())->SetStaticFriction(value);
	}

	float ScriptPhysicsMaterial::InternalGetStaticFriction(ScriptPhysicsMaterial* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PhysicsMaterial*>(self->GetNativeObject())->GetStaticFriction();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::InternalSetDynamicFriction(ScriptPhysicsMaterial* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PhysicsMaterial*>(self->GetNativeObject())->SetDynamicFriction(value);
	}

	float ScriptPhysicsMaterial::InternalGetDynamicFriction(ScriptPhysicsMaterial* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PhysicsMaterial*>(self->GetNativeObject())->GetDynamicFriction();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::InternalSetRestitutionCoefficient(ScriptPhysicsMaterial* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PhysicsMaterial*>(self->GetNativeObject())->SetRestitutionCoefficient(value);
	}

	float ScriptPhysicsMaterial::InternalGetRestitutionCoefficient(ScriptPhysicsMaterial* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PhysicsMaterial*>(self->GetNativeObject())->GetRestitutionCoefficient();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::InternalCreate(MonoObject* scriptObject, float staticFriction, float dynamicFriction, float restitution)
	{
		TResourceHandle<PhysicsMaterial> nativeObject = PhysicsMaterial::Create(staticFriction, dynamicFriction, restitution);
		ScriptObjectWrapper::Create<ScriptPhysicsMaterial>(nativeObject, scriptObject);
	}
}
