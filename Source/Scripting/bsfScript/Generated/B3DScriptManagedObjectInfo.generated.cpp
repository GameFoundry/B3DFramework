//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedObjectInfo.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptManagedObjectInfo.generated.h"
#include "B3DScriptManagedTypeInfoObject.generated.h"
#include "B3DScriptManagedMemberInfo.generated.h"

namespace b3d
{
	ScriptManagedObjectInfo::ScriptManagedObjectInfo(const TShared<ManagedObjectInfo>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedObjectInfo::~ScriptManagedObjectInfo()
	{
		UnregisterEvents();
	}

	void ScriptManagedObjectInfo::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetReflectionType", (void*)&ScriptManagedObjectInfo::InternalGetReflectionType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeInfo", (void*)&ScriptManagedObjectInfo::InternalGetTypeInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeInfo", (void*)&ScriptManagedObjectInfo::InternalSetTypeInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMembers", (void*)&ScriptManagedObjectInfo::InternalGetMembers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMembers", (void*)&ScriptManagedObjectInfo::InternalSetMembers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBaseClass", (void*)&ScriptManagedObjectInfo::InternalGetBaseClass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBaseClass", (void*)&ScriptManagedObjectInfo::InternalSetBaseClass);

	}

	MonoObject* ScriptManagedObjectInfo::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoReflectionType* ScriptManagedObjectInfo::InternalGetReflectionType(ScriptManagedObjectInfo* self)
	{
		_MonoReflectionType* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedObjectInfo*>(self->GetNativeObject())->GetReflectionType();

		MonoReflectionType* __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedObjectInfo::InternalGetTypeInfo(ScriptManagedObjectInfo* self)
	{
		TShared<ManagedTypeInfoObject> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedObjectInfo*>(self->GetNativeObject())->TypeInfo;

		MonoObject* __output;
		__output = ScriptManagedTypeInfoObject::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedObjectInfo::InternalSetTypeInfo(ScriptManagedObjectInfo* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ManagedTypeInfoObject> tmpvalue;
		ScriptManagedTypeInfoObject* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptManagedTypeInfoObject::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfoObject>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedObjectInfo*>(self->GetNativeObject())->TypeInfo = tmpvalue;
	}

	MonoArray* ScriptManagedObjectInfo::InternalGetMembers(ScriptManagedObjectInfo* self)
	{
		Vector<TShared<ManagedMemberInfo>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ManagedObjectInfo*>(self->GetNativeObject())->Members;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptManagedMemberInfo>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			TShared<ManagedMemberInfo> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptManagedMemberInfo::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptManagedObjectInfo::InternalSetMembers(ScriptManagedObjectInfo* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TShared<ManagedMemberInfo>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				TShared<ManagedMemberInfo> arrayElementPointervalue;
				ScriptManagedMemberInfoWrapperBase* scriptObjectWrappervalue;
				scriptObjectWrappervalue = (ScriptManagedMemberInfoWrapperBase*)ScriptManagedMemberInfo::GetScriptObjectWrapper(scriptArrayvalue.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappervalue != nullptr)
				{
					arrayElementPointervalue = std::static_pointer_cast<ManagedMemberInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
					nativeArrayvalue[elementIndex] = arrayElementPointervalue;
				}
			}

		}
		static_cast<ManagedObjectInfo*>(self->GetNativeObject())->Members = nativeArrayvalue;
	}

	MonoObject* ScriptManagedObjectInfo::InternalGetBaseClass(ScriptManagedObjectInfo* self)
	{
		TShared<ManagedObjectInfo> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedObjectInfo*>(self->GetNativeObject())->BaseClass;

		MonoObject* __output;
		__output = ScriptManagedObjectInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedObjectInfo::InternalSetBaseClass(ScriptManagedObjectInfo* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ManagedObjectInfo> tmpvalue;
		ScriptManagedObjectInfo* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptManagedObjectInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedObjectInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedObjectInfo*>(self->GetNativeObject())->BaseClass = tmpvalue;
	}
}
