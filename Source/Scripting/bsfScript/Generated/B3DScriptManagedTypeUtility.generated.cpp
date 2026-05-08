//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeUtility.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../B3DManagedTypeUtility.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptManagedTypeInfo.generated.h"
#include "B3DScriptManagedTypeInfoObject.generated.h"
#include "B3DScriptManagedObjectInfo.generated.h"
#include "B3DScriptManagedTypeInfoArray.generated.h"
#include "B3DScriptManagedTypeInfoDictionary.generated.h"
#include "B3DScriptManagedTypeInfoList.generated.h"

namespace b3d
{
	ScriptManagedTypeUtility::ScriptManagedTypeUtility()
		:TScriptTypeDefinition()
	{
	}

	void ScriptManagedTypeUtility::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeInfo", (void*)&ScriptManagedTypeUtility::InternalGetTypeInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSerializableObjectInfo", (void*)&ScriptManagedTypeUtility::InternalGetSerializableObjectInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRTTITypeId", (void*)&ScriptManagedTypeUtility::InternalGetRTTITypeId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateSerializableObject", (void*)&ScriptManagedTypeUtility::InternalCreateSerializableObject);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateArray", (void*)&ScriptManagedTypeUtility::InternalCreateArray);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateList", (void*)&ScriptManagedTypeUtility::InternalCreateList);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateDictionary", (void*)&ScriptManagedTypeUtility::InternalCreateDictionary);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CloneObject", (void*)&ScriptManagedTypeUtility::InternalCloneObject);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateObjectOfType", (void*)&ScriptManagedTypeUtility::InternalCreateObjectOfType);

	}

	MonoObject* ScriptManagedTypeUtility::InternalGetTypeInfo(MonoReflectionType* objectType)
	{
		TShared<ManagedTypeInfo> tmp__output;
		_MonoReflectionType* tmpobjectType = nullptr;
		tmpobjectType = objectType;
		tmp__output = ManagedTypeUtility::GetTypeInfo(tmpobjectType);

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalGetSerializableObjectInfo(MonoReflectionType* objectType)
	{
		TShared<ManagedObjectInfo> tmp__output;
		_MonoReflectionType* tmpobjectType = nullptr;
		tmpobjectType = objectType;
		tmp__output = ManagedTypeUtility::GetSerializableObjectInfo(tmpobjectType);

		MonoObject* __output;
		__output = ScriptManagedObjectInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	uint32_t ScriptManagedTypeUtility::InternalGetRTTITypeId(MonoReflectionType* objectType)
	{
		uint32_t tmp__output;
		_MonoReflectionType* tmpobjectType = nullptr;
		tmpobjectType = objectType;
		tmp__output = ManagedTypeUtility::GetRTTITypeId(tmpobjectType);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCreateSerializableObject(MonoObject* typeInfo)
	{
		_MonoObject* tmp__output = nullptr;
		TShared<ManagedTypeInfoObject> tmptypeInfo;
		ScriptManagedTypeInfoObject* scriptObjectWrappertypeInfo;
		scriptObjectWrappertypeInfo = ScriptManagedTypeInfoObject::GetScriptObjectWrapper(typeInfo);
		if(scriptObjectWrappertypeInfo != nullptr)
			tmptypeInfo = std::static_pointer_cast<ManagedTypeInfoObject>(scriptObjectWrappertypeInfo->GetBaseNativeObjectAsShared());
		tmp__output = ManagedTypeUtility::CreateSerializableObject(tmptypeInfo);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCreateArray(MonoObject* typeInfo, MonoArray* arraySizes)
	{
		_MonoObject* tmp__output = nullptr;
		TShared<ManagedTypeInfoArray> tmptypeInfo;
		ScriptManagedTypeInfoArray* scriptObjectWrappertypeInfo;
		scriptObjectWrappertypeInfo = ScriptManagedTypeInfoArray::GetScriptObjectWrapper(typeInfo);
		if(scriptObjectWrappertypeInfo != nullptr)
			tmptypeInfo = std::static_pointer_cast<ManagedTypeInfoArray>(scriptObjectWrappertypeInfo->GetBaseNativeObjectAsShared());
		Vector<uint32_t> nativeArrayarraySizes;
		if(arraySizes != nullptr)
		{
			ScriptArray scriptArrayarraySizes(arraySizes);
			nativeArrayarraySizes.resize(scriptArrayarraySizes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayarraySizes.Size(); elementIndex++)
			{
				nativeArrayarraySizes[elementIndex] = scriptArrayarraySizes.Get<uint32_t>(elementIndex);
			}
		}
		tmp__output = ManagedTypeUtility::CreateArray(tmptypeInfo, nativeArrayarraySizes);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCreateList(MonoObject* typeInfo, uint32_t size)
	{
		_MonoObject* tmp__output = nullptr;
		TShared<ManagedTypeInfoList> tmptypeInfo;
		ScriptManagedTypeInfoList* scriptObjectWrappertypeInfo;
		scriptObjectWrappertypeInfo = ScriptManagedTypeInfoList::GetScriptObjectWrapper(typeInfo);
		if(scriptObjectWrappertypeInfo != nullptr)
			tmptypeInfo = std::static_pointer_cast<ManagedTypeInfoList>(scriptObjectWrappertypeInfo->GetBaseNativeObjectAsShared());
		tmp__output = ManagedTypeUtility::CreateList(tmptypeInfo, size);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCreateDictionary(MonoObject* typeInfo)
	{
		_MonoObject* tmp__output = nullptr;
		TShared<ManagedTypeInfoDictionary> tmptypeInfo;
		ScriptManagedTypeInfoDictionary* scriptObjectWrappertypeInfo;
		scriptObjectWrappertypeInfo = ScriptManagedTypeInfoDictionary::GetScriptObjectWrapper(typeInfo);
		if(scriptObjectWrappertypeInfo != nullptr)
			tmptypeInfo = std::static_pointer_cast<ManagedTypeInfoDictionary>(scriptObjectWrappertypeInfo->GetBaseNativeObjectAsShared());
		tmp__output = ManagedTypeUtility::CreateDictionary(tmptypeInfo);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCloneObject(MonoObject* original)
	{
		_MonoObject* tmp__output = nullptr;
		_MonoObject* tmporiginal = nullptr;
		tmporiginal = original;
		tmp__output = ManagedTypeUtility::CloneObject(tmporiginal);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCreateObjectOfType(MonoReflectionType* type)
	{
		_MonoObject* tmp__output = nullptr;
		_MonoReflectionType* tmptype = nullptr;
		tmptype = type;
		tmp__output = ManagedTypeUtility::CreateObjectOfType(tmptype);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}
}
