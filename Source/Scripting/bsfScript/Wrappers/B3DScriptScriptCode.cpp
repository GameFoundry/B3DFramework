//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptScriptCode.h"
#include "B3DScriptResourceManager.h"
#include "B3DMonoField.h"
#include "B3DMonoClass.h"
#include "B3DMonoManager.h"
#include "B3DMonoUtil.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Serialization/B3DManagedTypeInfo.h"
#include <regex>

using namespace b3d;
ScriptScriptCode::ScriptScriptCode(const HScriptCode& nativeObject)
	: TScriptResourceWrapper(nativeObject)
{
	RegisterEvents();
}

void ScriptScriptCode::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptScriptCode::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptScriptCode::InternalGetText);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptScriptCode::InternalSetText);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsEditorScript", (void*)&ScriptScriptCode::InternalIsEditorScript);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEditorScript", (void*)&ScriptScriptCode::InternalSetEditorScript);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypes", (void*)&ScriptScriptCode::InternalGetTypes);
}

MonoObject* ScriptScriptCode::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

ScriptCode* ScriptScriptCode::GetNativeObject() const
{
	return static_cast<ScriptCode*>(TScriptResourceWrapper::GetNativeObject());
}


void ScriptScriptCode::InternalCreateInstance(MonoObject* scriptObject, MonoString* text)
{
	WString strText = MonoUtil::MonoToWString(text);
	HScriptCode scriptCode = ScriptCode::Create(strText);

	ScriptObjectWrapper::Create<ScriptScriptCode>(scriptCode, scriptObject);
}

MonoString* ScriptScriptCode::InternalGetText(ScriptScriptCode* self)
{
	if(!self->IsNativeObjectValid())
		MonoUtil::WstringToMono(L"");

	return MonoUtil::WstringToMono(self->GetNativeObject()->GetString());
}

void ScriptScriptCode::InternalSetText(ScriptScriptCode* self, MonoString* text)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetString(MonoUtil::MonoToWString(text));
}

bool ScriptScriptCode::InternalIsEditorScript(ScriptScriptCode* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->GetIsEditorScript();
}

void ScriptScriptCode::InternalSetEditorScript(ScriptScriptCode* self, bool value)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetIsEditorScript(value);
}

MonoArray* ScriptScriptCode::InternalGetTypes(ScriptScriptCode* self)
{
	Vector<FullTypeName> types;
	if(self->IsNativeObjectValid())
		types = ParseTypes(self->GetNativeObject()->GetString());

	Vector<MonoReflectionType*> validTypes;
	for(auto& type : types)
	{
		TShared<ManagedObjectInfo> objInfo;
		if(ScriptAssemblyManager::Instance().GetSerializableObjectInfo(ToString(type.first), ToString(type.second), objInfo))
			validTypes.push_back(MonoUtil::GetType(objInfo->TypeInfo->GetMonoClass()));
	}

	u32 numValidTypes = (u32)validTypes.size();
	MonoClass* typeClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemTypeClass;

	ScriptArray scriptArray(typeClass->GetInternalClass(), numValidTypes);
	for(u32 i = 0; i < numValidTypes; i++)
		scriptArray.Set(i, validTypes[i]);

	return scriptArray.GetInternal();
}

Vector<ScriptScriptCode::FullTypeName> ScriptScriptCode::ParseTypes(const WString& code)
{
	struct NamespaceData
	{
		WString Ns;
		i32 BracketIdx;
	};

	Vector<FullTypeName> output;
	Stack<NamespaceData> namespaces;

	// TODO: Won't match non latin characters because C++ regex doesn't support unicode character classes
	// and writing out Unicode ranges for all the characters C# supports as identifiers is too tedious at the moment.
	// Classes that need to match: \p{Lu}\p{Ll}\p{Lt}\p{Lm}\p{Lo}\p{Nl}\p{Mn}\p{Mc}\p{Nd}\p{Pc}\p{Cf}
	WString identifierPattern = LR"([_@a-zA-Z][_\da-zA-Z]*)";
	std::wregex identifierRegex(identifierPattern);

	WString nsToken = L"namespace";
	WString classToken = L"class";

	u32 idx = 0;
	i32 bracketIdx = 0;
	for(auto iter = code.begin(); iter != code.end(); ++iter)
	{
		wchar_t ch = *iter;

		if(code.compare(idx, classToken.size(), classToken) == 0)
		{
			std::match_results<WString::const_iterator> results;
			if(std::regex_search(iter + classToken.size(), code.end(), results, identifierRegex))
			{
				WString ns = L"";
				if(!namespaces.empty())
					ns = namespaces.top().Ns;

				std::wstring tempStr = results[0];
				WString typeName = tempStr.c_str();

				output.push_back(FullTypeName());
				FullTypeName& nsTypePair = output.back();
				nsTypePair.first = ns;
				nsTypePair.second = typeName;
			}
		}
		else if(code.compare(idx, nsToken.size(), nsToken) == 0)
		{
			std::match_results<WString::const_iterator> results;
			if(std::regex_search(iter + nsToken.size(), code.end(), results, identifierRegex))
			{
				std::wstring tempStr = results[0];
				WString ns = tempStr.c_str();

				namespaces.push({ ns, bracketIdx + 1 });
			}
		}
		else if(ch == '{')
		{
			bracketIdx++;
		}
		else if(ch == '}')
		{
			bracketIdx--;
		}

		idx++;
	}

	return output;
}
