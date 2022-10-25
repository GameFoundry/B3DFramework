//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptScriptCode.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include <regex>

using namespace std::placeholders;

namespace bs
{
	ScriptScriptCode::ScriptScriptCode(MonoObject* instance, const HScriptCode& scriptCode)
		: TScriptResource(instance, scriptCode)
	{
	}

	void ScriptScriptCode::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptScriptCode::InternalCreateInstance);
		metaData.ScriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptScriptCode::InternalGetText);
		metaData.ScriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptScriptCode::InternalSetText);
		metaData.ScriptClass->AddInternalCall("Internal_IsEditorScript", (void*)&ScriptScriptCode::InternalIsEditorScript);
		metaData.ScriptClass->AddInternalCall("Internal_SetEditorScript", (void*)&ScriptScriptCode::InternalSetEditorScript);
		metaData.ScriptClass->AddInternalCall("Internal_GetTypes", (void*)&ScriptScriptCode::InternalGetTypes);
	}

	void ScriptScriptCode::InternalCreateInstance(MonoObject* instance, MonoString* text)
	{
		WString strText = MonoUtil::MonoToWString(text);
		HScriptCode scriptCode = ScriptCode::Create(strText);

		ScriptResourceManager::Instance().CreateBuiltinScriptResource(scriptCode, instance);
	}

	MonoString* ScriptScriptCode::InternalGetText(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if(!scriptCode.IsLoaded())
			MonoUtil::WstringToMono(L"");

		return MonoUtil::WstringToMono(scriptCode->GetString());
	}

	void ScriptScriptCode::InternalSetText(ScriptScriptCode* thisPtr, MonoString* text)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if(!scriptCode.IsLoaded())
			return;

		scriptCode->SetString(MonoUtil::MonoToWString(text));
	}

	bool ScriptScriptCode::InternalIsEditorScript(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if(!scriptCode.IsLoaded())
			return false;

		return scriptCode->GetIsEditorScript();
	}

	void ScriptScriptCode::InternalSetEditorScript(ScriptScriptCode* thisPtr, bool value)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if(!scriptCode.IsLoaded())
			return;

		scriptCode->SetIsEditorScript(value);
	}

	MonoArray* ScriptScriptCode::InternalGetTypes(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();

		Vector<FullTypeName> types;
		if(scriptCode.IsLoaded())
			types = ParseTypes(scriptCode->GetString());

		Vector<MonoReflectionType*> validTypes;
		for(auto& type : types)
		{
			SPtr<ManagedSerializableObjectInfo> objInfo;
			if(ScriptAssemblyManager::Instance().GetSerializableObjectInfo(toString(type.first), toString(type.second), objInfo))
				validTypes.push_back(MonoUtil::GetType(objInfo->MTypeInfo->GetMonoClass()));
		}

		u32 numValidTypes = (u32)validTypes.size();
		MonoClass* typeClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemTypeClass;

		ScriptArray scriptArray(typeClass->GetInternalClassInternal(), numValidTypes);
		for(u32 i = 0; i < numValidTypes; i++)
			scriptArray.Set(i, validTypes[i]);

		return scriptArray.GetInternal();
	}

	MonoObject* ScriptScriptCode::CreateInstance()
	{
		return metaData.ScriptClass->CreateInstance();
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
} // namespace bs
