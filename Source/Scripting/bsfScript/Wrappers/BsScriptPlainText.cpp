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
		: TScriptResource(instance, plainText)
	{
	}

	void ScriptPlainText::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptPlainText::InternalCreateInstance);
		metaData.ScriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptPlainText::InternalGetText);
		metaData.ScriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptPlainText::InternalSetText);
	}

	void ScriptPlainText::InternalCreateInstance(MonoObject* instance, MonoString* text)
	{
		WString strText = MonoUtil::MonoToWString(text);
		HPlainText plainText = PlainText::Create(strText);

		ScriptResourceManager::Instance().CreateBuiltinScriptResource(plainText, instance);
	}

	MonoString* ScriptPlainText::InternalGetText(ScriptPlainText* thisPtr)
	{
		HPlainText plainText = thisPtr->GetHandle();

		return MonoUtil::WstringToMono(plainText->GetString());
	}

	void ScriptPlainText::InternalSetText(ScriptPlainText* thisPtr, MonoString* text)
	{
		HPlainText plainText = thisPtr->GetHandle();

		plainText->SetString(MonoUtil::MonoToWString(text));
	}

	MonoObject* ScriptPlainText::CreateInstance()
	{
		return metaData.ScriptClass->CreateInstance();
	}
} // namespace bs
