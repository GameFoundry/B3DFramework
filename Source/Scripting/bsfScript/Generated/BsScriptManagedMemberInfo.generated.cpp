//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedMemberInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptManagedMemberStyle.generated.h"
#include "BsScriptManagedTypeInfo.generated.h"

namespace bs
{
	ScriptManagedMemberInfo::ScriptManagedMemberInfo(const SPtr<ManagedMemberInfo>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedMemberInfo::~ScriptManagedMemberInfo()
	{
		UnregisterEvents();
	}

	void ScriptManagedMemberInfo::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsSerializable", (void*)&ScriptManagedMemberInfo::InternalIsSerializable);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ParseStyle", (void*)&ScriptManagedMemberInfo::InternalParseStyle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptManagedMemberInfo::InternalGetValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptManagedMemberInfo::InternalSetValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptManagedMemberInfo::InternalGetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptManagedMemberInfo::InternalSetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeInfo", (void*)&ScriptManagedMemberInfo::InternalGetTypeInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeInfo", (void*)&ScriptManagedMemberInfo::InternalSetTypeInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMetaDataFlags", (void*)&ScriptManagedMemberInfo::InternalGetMetaDataFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMetaDataFlags", (void*)&ScriptManagedMemberInfo::InternalSetMetaDataFlags);

	}

	MonoObject* ScriptManagedMemberInfo::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	bool ScriptManagedMemberInfo::InternalIsSerializable(ScriptManagedMemberInfoWrapperBase* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedMemberInfo*>(self->GetNativeObject())->IsSerializable();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedMemberInfo::InternalParseStyle(ScriptManagedMemberInfoWrapperBase* self, __ManagedMemberStyleInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ManagedMemberStyle tmp__output;
		tmp__output = static_cast<ManagedMemberInfo*>(self->GetNativeObject())->ParseStyle();

		__ManagedMemberStyleInterop interop__output;
		interop__output = ScriptManagedMemberStyle::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptManagedMemberStyle::GetMetaData()->ScriptClass->GetInternalClass());
	}

	MonoObject* ScriptManagedMemberInfo::InternalGetValue(ScriptManagedMemberInfoWrapperBase* self, MonoObject* instance)
	{
		_MonoObject* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		_MonoObject* tmpinstance = nullptr;
		tmpinstance = instance;
		tmp__output = static_cast<ManagedMemberInfo*>(self->GetNativeObject())->GetValue(tmpinstance);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedMemberInfo::InternalSetValue(ScriptManagedMemberInfoWrapperBase* self, MonoObject* instance, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		_MonoObject* tmpinstance = nullptr;
		tmpinstance = instance;
		_MonoObject* tmpvalue = nullptr;
		tmpvalue = value;
		static_cast<ManagedMemberInfo*>(self->GetNativeObject())->SetValue(tmpinstance, tmpvalue);
	}

	MonoString* ScriptManagedMemberInfo::InternalGetName(ScriptManagedMemberInfoWrapperBase* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedMemberInfo*>(self->GetNativeObject())->Name;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedMemberInfo::InternalSetName(ScriptManagedMemberInfoWrapperBase* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedMemberInfo*>(self->GetNativeObject())->Name = tmpvalue;
	}

	MonoObject* ScriptManagedMemberInfo::InternalGetTypeInfo(ScriptManagedMemberInfoWrapperBase* self)
	{
		SPtr<ManagedTypeInfo> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedMemberInfo*>(self->GetNativeObject())->TypeInfo;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedMemberInfo::InternalSetTypeInfo(ScriptManagedMemberInfoWrapperBase* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		SPtr<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedMemberInfo*>(self->GetNativeObject())->TypeInfo = tmpvalue;
	}

	ManagedFieldMetaDataFlag ScriptManagedMemberInfo::InternalGetMetaDataFlags(ScriptManagedMemberInfoWrapperBase* self)
	{
		Flags<ManagedFieldMetaDataFlag> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedMemberInfo*>(self->GetNativeObject())->MetaDataFlags;

		ManagedFieldMetaDataFlag __output;
		__output = (ManagedFieldMetaDataFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptManagedMemberInfo::InternalSetMetaDataFlags(ScriptManagedMemberInfoWrapperBase* self, ManagedFieldMetaDataFlag value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedMemberInfo*>(self->GetNativeObject())->MetaDataFlags = value;
	}
}
