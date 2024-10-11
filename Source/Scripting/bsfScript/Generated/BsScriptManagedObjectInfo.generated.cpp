//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedObjectInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptManagedObjectInfo.generated.h"
#include "BsScriptManagedTypeInfoObject.generated.h"
#include "BsScriptManagedMemberInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedFieldInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedPropertyInfo.generated.h"

namespace bs
{
	ScriptManagedObjectInfo::ScriptManagedObjectInfo(const SPtr<ManagedObjectInfo>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptManagedObjectInfo::SetupScriptBindings()
	{
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
	MonoObject* ScriptManagedObjectInfo::InternalGetTypeInfo(ScriptManagedObjectInfo* self)
	{
		SPtr<ManagedTypeInfoObject> tmp__output;
		tmp__output = static_cast<ManagedObjectInfo*>(self->GetNativeObject())->TypeInfo;

		MonoObject* __output;
		__output = ScriptManagedTypeInfoObject::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedObjectInfo::InternalSetTypeInfo(ScriptManagedObjectInfo* self, MonoObject* value)
	{
		SPtr<ManagedTypeInfoObject> tmpvalue;
		ScriptManagedTypeInfoObject* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptManagedTypeInfoObject::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfoObject>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedObjectInfo*>(self->GetNativeObject())->TypeInfo = tmpvalue;
	}

	MonoArray* ScriptManagedObjectInfo::InternalGetMembers(ScriptManagedObjectInfo* self)
	{
		Vector<SPtr<ManagedMemberInfo>> nativeArray__output;
		nativeArray__output = static_cast<ManagedObjectInfo*>(self->GetNativeObject())->Members;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptManagedMemberInfo>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<ManagedMemberInfo> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptManagedMemberInfo::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptManagedObjectInfo::InternalSetMembers(ScriptManagedObjectInfo* self, MonoArray* value)
	{
		Vector<SPtr<ManagedMemberInfo>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				ScriptManagedMemberInfoWrapperBase* scriptObjectWrappervalue;
				scriptObjectWrappervalue = (ScriptManagedMemberInfoWrapperBase*)ScriptManagedMemberInfo::GetScriptObjectWrapper(scriptArrayvalue.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappervalue != nullptr)
				{
					SPtr<ManagedMemberInfo> arrayElementPointervalue = std::static_pointer_cast<ManagedMemberInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
					nativeArrayvalue[elementIndex] = arrayElementPointervalue;
				}
			}

		}
		static_cast<ManagedObjectInfo*>(self->GetNativeObject())->Members = nativeArrayvalue;
	}

	MonoObject* ScriptManagedObjectInfo::InternalGetBaseClass(ScriptManagedObjectInfo* self)
	{
		SPtr<ManagedObjectInfo> tmp__output;
		tmp__output = static_cast<ManagedObjectInfo*>(self->GetNativeObject())->BaseClass;

		MonoObject* __output;
		__output = ScriptManagedObjectInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedObjectInfo::InternalSetBaseClass(ScriptManagedObjectInfo* self, MonoObject* value)
	{
		SPtr<ManagedObjectInfo> tmpvalue;
		ScriptManagedObjectInfo* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptManagedObjectInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedObjectInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedObjectInfo*>(self->GetNativeObject())->BaseClass = tmpvalue;
	}
}
