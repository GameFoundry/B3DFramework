//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoPrimitive.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptManagedTypeInfoPrimitive::ScriptManagedTypeInfoPrimitive(const TShared<ManagedTypeInfoPrimitive>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoPrimitive::~ScriptManagedTypeInfoPrimitive()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoPrimitive::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPrimitiveType", (void*)&ScriptManagedTypeInfoPrimitive::InternalGetPrimitiveType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPrimitiveType", (void*)&ScriptManagedTypeInfoPrimitive::InternalSetPrimitiveType);

	}

	MonoObject* ScriptManagedTypeInfoPrimitive::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	ManagedPrimitiveType ScriptManagedTypeInfoPrimitive::InternalGetPrimitiveType(ScriptManagedTypeInfoPrimitive* self)
	{
		ManagedPrimitiveType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoPrimitive*>(self->GetNativeObject())->PrimitiveType;

		ManagedPrimitiveType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoPrimitive::InternalSetPrimitiveType(ScriptManagedTypeInfoPrimitive* self, ManagedPrimitiveType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoPrimitive*>(self->GetNativeObject())->PrimitiveType = value;
	}
}
