//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptStringTable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace b3d
{
	ScriptStringTable::ScriptStringTable(const TResourceHandle<StringTable>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptStringTable::~ScriptStringTable()
	{
		UnregisterEvents();
	}

	void ScriptStringTable::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptStringTable::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Contains", (void*)&ScriptStringTable::InternalContains);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNumStrings", (void*)&ScriptStringTable::InternalGetNumStrings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIdentifiers", (void*)&ScriptStringTable::InternalGetIdentifiers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetString", (void*)&ScriptStringTable::InternalSetString);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetString", (void*)&ScriptStringTable::InternalGetString);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveString", (void*)&ScriptStringTable::InternalRemoveString);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptStringTable::InternalCreate);

	}

	MonoObject* ScriptStringTable::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptStringTable::InternalGetRef(ScriptStringTable* self)
	{
		return self->GetOrCreateResourceReference();
	}

	bool ScriptStringTable::InternalContains(ScriptStringTable* self, MonoString* identifier)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		tmp__output = static_cast<StringTable*>(self->GetNativeObject())->Contains(tmpidentifier);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptStringTable::InternalGetNumStrings(ScriptStringTable* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<StringTable*>(self->GetNativeObject())->GetNumStrings();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptStringTable::InternalGetIdentifiers(ScriptStringTable* self)
	{
		Vector<String> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<StringTable*>(self->GetNativeObject())->GetIdentifiers();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<String>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptStringTable::InternalSetString(ScriptStringTable* self, MonoString* identifier, Language language, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<StringTable*>(self->GetNativeObject())->SetString(tmpidentifier, language, tmpvalue);
	}

	MonoString* ScriptStringTable::InternalGetString(ScriptStringTable* self, MonoString* identifier, Language language)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		tmp__output = static_cast<StringTable*>(self->GetNativeObject())->GetString(tmpidentifier, language);

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptStringTable::InternalRemoveString(ScriptStringTable* self, MonoString* identifier)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		static_cast<StringTable*>(self->GetNativeObject())->RemoveString(tmpidentifier);
	}

	void ScriptStringTable::InternalCreate(MonoObject* scriptObject)
	{
		TResourceHandle<StringTable> nativeObject = StringTable::Create();
		ScriptObjectWrapper::Create<ScriptStringTable>(nativeObject, scriptObject);
	}
}
