//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPlainText.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsPlainText.h"

using namespace std::placeholders;

namespace bs
{
	ScriptPlainText::ScriptPlainText(MonoObject* instance, const HPlainText& plainText)
		:TScriptResource(instance, plainText)
	{

	}

	void ScriptPlainText::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptPlainText::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetText", &ScriptPlainText::internal_getText);
		metaData.scriptClass->addInternalCall("Internal_SetText", &ScriptPlainText::internal_setText);
	}

	void ScriptPlainText::internal_createInstance(MonoObject* instance, MonoString* text)
	{
		WString strText = MonoUtil::monoToWString(text);
		HPlainText plainText = PlainText::create(strText);

		ScriptPlainText* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, plainText, &scriptInstance);
	}

	MonoString* ScriptPlainText::internal_getText(ScriptPlainText* thisPtr)
	{
		HPlainText plainText = thisPtr->getHandle();

		return MonoUtil::wstringToMono(plainText->getString());
	}

	void ScriptPlainText::internal_setText(ScriptPlainText* thisPtr, MonoString* text)
	{
		HPlainText plainText = thisPtr->getHandle();

		plainText->setString(MonoUtil::monoToWString(text));
	}

	MonoObject* ScriptPlainText::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}