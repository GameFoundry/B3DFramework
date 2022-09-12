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
		:TScriptResource(instance, scriptCode)
	{
		
	}

	void ScriptScriptCode::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptScriptCode::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptScriptCode::internal_getText);
		metaData.scriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptScriptCode::internal_setText);
		metaData.scriptClass->AddInternalCall("Internal_IsEditorScript", (void*)&ScriptScriptCode::internal_isEditorScript);
		metaData.scriptClass->AddInternalCall("Internal_SetEditorScript", (void*)&ScriptScriptCode::internal_setEditorScript);
		metaData.scriptClass->AddInternalCall("Internal_GetTypes", (void*)&ScriptScriptCode::internal_getTypes);
	}

	void ScriptScriptCode::internal_createInstance(MonoObject* instance, MonoString* text)
	{
		WString strText = MonoUtil::monoToWString(text);
		HScriptCode scriptCode = ScriptCode::create(strText);

		ScriptResourceManager::instance().CreateBuiltinScriptResource(scriptCode, instance);
	}

	MonoString* ScriptScriptCode::internal_getText(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if (!scriptCode.IsLoaded())
			MonoUtil::wstringToMono(L"");

		return MonoUtil::WstringToMono(scriptCode->GetString());
	}

	void ScriptScriptCode::internal_setText(ScriptScriptCode* thisPtr, MonoString* text)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if (!scriptCode.IsLoaded())
			return;

		scriptCode->SetString(MonoUtil::monoToWString(text));
	}

	bool ScriptScriptCode::internal_isEditorScript(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if (!scriptCode.IsLoaded())
			return false;

		return scriptCode->GetIsEditorScript();
	}

	void ScriptScriptCode::internal_setEditorScript(ScriptScriptCode* thisPtr, bool value)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();
		if (!scriptCode.IsLoaded())
			return;

		scriptCode->SetIsEditorScript(value);
	}
	
	MonoArray* ScriptScriptCode::internal_getTypes(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->GetHandle();

		Vector<FullTypeName> types;
		if (scriptCode.IsLoaded())
			types = parseTypes(scriptCode->GetString());

		Vector<MonoReflectionType*> validTypes;
		for (auto& type : types)
		{
			SPtr<ManagedSerializableObjectInfo> objInfo;
			if (ScriptAssemblyManager::instance().GetSerializableObjectInfo(toString(type.first), toString(type.second), objInfo))
				validTypes.push_back(MonoUtil::getType(objInfo->mTypeInfo->GetMonoClass()));
		}

		UINT32 numValidTypes = (UINT32)validTypes.Size();
		MonoClass* typeClass = ScriptAssemblyManager::instance().GetBuiltinClasses().systemTypeClass;

		ScriptArray ScriptArray(typeClass->_getInternalClass(), numValidTypes);
		for (UINT32 i = 0; i < numValidTypes; i++)
			scriptArray.Set(i, validTypes[i]);

		return scriptArray.GetInternal();
	}

	MonoObject* ScriptScriptCode::createInstance()
	{
		return metaData.scriptClass->CreateInstance();
	}

	Vector<ScriptScriptCode::FullTypeName> ScriptScriptCode::ParseTypes(const WString& code)
	{
		struct NamespaceData
		{
			WString ns;
			INT32 bracketIdx;
		};

		Vector<FullTypeName> output;
		Stack<NamespaceData> namespaces;

		// TODO: Won't match non latin characters because C++ regex doesn't support unicode character classes
		// and writing out Unicode ranges for all the characters C# supports as identifiers is too tedious at the moment.
		// Classes that need to match: \p{Lu}\p{Ll}\p{Lt}\p{Lm}\p{Lo}\p{Nl}\p{Mn}\p{Mc}\p{Nd}\p{Pc}\p{Cf}
		WString identifierPattern = LR"([_@a-zA-Z][_\da-zA-Z]*)";
		std::wregex IdentifierRegex(identifierPattern);

		WString nsToken = L"namespace";
		WString classToken = L"class";

		UINT32 idx = 0;
		INT32 bracketIdx = 0;
		for (auto iter = code.Begin(); iter != code.end(); ++iter)
		{
			wchar_t ch = *iter;
			
			if (code.Compare(idx, classToken.size(), classToken) == 0)
			{
				std::match_results<WString::const_iterator> results;
				if (std::regex_search(iter + classToken.Size(), code.end(), results, identifierRegex))
				{
					WString ns = L"";
					if (!namespaces.Empty())
						ns = namespaces.Top().ns;

					std::wstring tempStr = results[0];
					WString typeName = tempStr.c_str();

					output.push_back(FullTypeName());
					FullTypeName& nsTypePair = output.Back();
					nsTypePair.first = ns;
					nsTypePair.second = typeName;
				}
			}
			else if (code.Compare(idx, nsToken.size(), nsToken) == 0)
			{
				std::match_results<WString::const_iterator> results;
				if (std::regex_search(iter + nsToken.Size(), code.end(), results, identifierRegex))
				{
					std::wstring tempStr = results[0];
					WString ns = tempStr.c_str();

					namespaces.Push({ ns, bracketIdx + 1 });
				}
			}
			else if (ch == '{')
			{
				bracketIdx++;
			}
			else if (ch == '}')
			{
				bracketIdx--;
			}

			idx++;
		}

		return output;
	}
}
