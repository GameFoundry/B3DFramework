//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptPlainText.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Resources/BsPlainText.h"

using namespace std::placeholders;

namespace bs
{
	ScriptPlainText::ScriptPlainText(MonoObject* instance, const HPlainText& plainText)
		:TScriptResource(instance, plainText)
	{

	}

	void ScriptPlainText::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptPlainText::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptPlainText::internal_getText);
		metaData.scriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptPlainText::internal_setText);
	}

	void ScriptPlainText::internal_createInstance(MonoObject* instance, MonoString* text)
	{
		WString strText = MonoUtil::monoToWString(text);
		HPlainText plainText = PlainText::create(strText);

		ScriptResourceManager::instance().CreateBuiltinScriptResource(plainText, instance);
	}

	MonoString* ScriptPlainText::internal_getText(ScriptPlainText* thisPtr)
	{
		HPlainText plainText = thisPtr->GetHandle();

		return MonoUtil::WstringToMono(plainText->GetString());
	}

	void ScriptPlainText::internal_setText(ScriptPlainText* thisPtr, MonoString* text)
	{
		HPlainText plainText = thisPtr->GetHandle();

		plainText->SetString(MonoUtil::monoToWString(text));
	}

	MonoObject* ScriptPlainText::createInstance()
	{
		return metaData.scriptClass->CreateInstance();
	}
}
