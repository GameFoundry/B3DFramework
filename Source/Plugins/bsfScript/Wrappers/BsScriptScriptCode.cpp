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

	void ScriptScriptCode::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptScriptCode::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetText", (void*)&ScriptScriptCode::internal_getText);
		metaData.scriptClass->addInternalCall("Internal_SetText", (void*)&ScriptScriptCode::internal_setText);
		metaData.scriptClass->addInternalCall("Internal_IsEditorScript", (void*)&ScriptScriptCode::internal_isEditorScript);
		metaData.scriptClass->addInternalCall("Internal_SetEditorScript", (void*)&ScriptScriptCode::internal_setEditorScript);
		metaData.scriptClass->addInternalCall("Internal_GetTypes", (void*)&ScriptScriptCode::internal_getTypes);
	}

	void ScriptScriptCode::internal_createInstance(MonoObject* instance, MonoString* text)
	{
		WString strText = MonoUtil::monoToWString(text);
		HScriptCode scriptCode = ScriptCode::create(strText);

		ScriptResourceManager::instance().createBuiltinScriptResource(scriptCode, instance);
	}

	MonoString* ScriptScriptCode::internal_getText(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->getHandle();
		if (!scriptCode.isLoaded())
			MonoUtil::wstringToMono(L"");

		return MonoUtil::wstringToMono(scriptCode->getString());
	}

	void ScriptScriptCode::internal_setText(ScriptScriptCode* thisPtr, MonoString* text)
	{
		HScriptCode scriptCode = thisPtr->getHandle();
		if (!scriptCode.isLoaded())
			return;

		scriptCode->setString(MonoUtil::monoToWString(text));
	}

	bool ScriptScriptCode::internal_isEditorScript(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->getHandle();
		if (!scriptCode.isLoaded())
			return false;

		return scriptCode->getIsEditorScript();
	}

	void ScriptScriptCode::internal_setEditorScript(ScriptScriptCode* thisPtr, bool value)
	{
		HScriptCode scriptCode = thisPtr->getHandle();
		if (!scriptCode.isLoaded())
			return;

		scriptCode->setIsEditorScript(value);
	}
	
	MonoArray* ScriptScriptCode::internal_getTypes(ScriptScriptCode* thisPtr)
	{
		HScriptCode scriptCode = thisPtr->getHandle();

		Vector<FullTypeName> types;
		if (scriptCode.isLoaded())
			types = parseTypes(scriptCode->getString());

		Vector<MonoReflectionType*> validTypes;
		for (auto& type : types)
		{
			SPtr<ManagedSerializableObjectInfo> objInfo;
			if (ScriptAssemblyManager::instance().getSerializableObjectInfo(toString(type.first), toString(type.second), objInfo))
				validTypes.push_back(MonoUtil::getType(objInfo->mTypeInfo->getMonoClass()));
		}

		UINT32 numValidTypes = (UINT32)validTypes.size();
		MonoClass* typeClass = ScriptAssemblyManager::instance().getBuiltinClasses().systemTypeClass;

		ScriptArray scriptArray(typeClass->_getInternalClass(), numValidTypes);
		for (UINT32 i = 0; i < numValidTypes; i++)
			scriptArray.set(i, validTypes[i]);

		return scriptArray.getInternal();
	}

	MonoObject* ScriptScriptCode::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}

	Vector<ScriptScriptCode::FullTypeName> ScriptScriptCode::parseTypes(const WString& code)
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
		std::wregex identifierRegex(identifierPattern);

		WString nsToken = L"namespace";
		WString classToken = L"class";

		UINT32 idx = 0;
		INT32 bracketIdx = 0;
		for (auto iter = code.begin(); iter != code.end(); ++iter)
		{
			wchar_t ch = *iter;
			
			if (code.compare(idx, classToken.size(), classToken) == 0)
			{
				std::match_results<WString::const_iterator> results;
				if (std::regex_search(iter + classToken.size(), code.end(), results, identifierRegex))
				{
					WString ns = L"";
					if (!namespaces.empty())
						ns = namespaces.top().ns;

					std::wstring tempStr = results[0];
					WString typeName = tempStr.c_str();

					output.push_back(FullTypeName());
					FullTypeName& nsTypePair = output.back();
					nsTypePair.first = ns;
					nsTypePair.second = typeName;
				}
			}
			else if (code.compare(idx, nsToken.size(), nsToken) == 0)
			{
				std::match_results<WString::const_iterator> results;
				if (std::regex_search(iter + nsToken.size(), code.end(), results, identifierRegex))
				{
					std::wstring tempStr = results[0];
					WString ns = tempStr.c_str();

					namespaces.push({ ns, bracketIdx + 1 });
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