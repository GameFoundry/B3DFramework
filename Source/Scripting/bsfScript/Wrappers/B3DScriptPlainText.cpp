//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptPlainText.h"
#include "B3DScriptResourceManager.h"
#include "B3DMonoField.h"
#include "B3DMonoClass.h"
#include "B3DMonoManager.h"
#include "B3DMonoUtil.h"
#include "Resources/B3DPlainText.h"

using namespace b3d;
ScriptPlainText::ScriptPlainText(const HPlainText& nativeObject)
	: TScriptResourceWrapper(nativeObject)
{
	RegisterEvents();
}

void ScriptPlainText::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptPlainText::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptPlainText::InternalGetText);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptPlainText::InternalSetText);
}

MonoObject* ScriptPlainText::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

PlainText* ScriptPlainText::GetNativeObject() const
{
	return static_cast<PlainText*>(TScriptResourceWrapper::GetNativeObject());
}

void ScriptPlainText::InternalCreateInstance(MonoObject* scriptObject, MonoString* text)
{
	WString wideString = MonoUtil::MonoToWString(text);
	HPlainText plainText = PlainText::Create(wideString);

	ScriptObjectWrapper::Create<ScriptPlainText>(plainText, scriptObject);
}

MonoString* ScriptPlainText::InternalGetText(ScriptPlainText* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	return MonoUtil::WstringToMono(self->GetNativeObject()->GetString());
}

void ScriptPlainText::InternalSetText(ScriptPlainText* self, MonoString* text)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetString(MonoUtil::MonoToWString(text));
}
