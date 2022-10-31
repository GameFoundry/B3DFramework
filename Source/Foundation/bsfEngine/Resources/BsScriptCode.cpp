//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsScriptCode.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsScriptCodeRTTI.h"

using namespace bs;

ScriptCode::ScriptCode(const WString& data, bool editorScript)
	: Resource(false), mString(data), mEditorScript(editorScript)
{
}

HScriptCode ScriptCode::Create(const WString& data, bool editorScript)
{
	return static_resource_cast<ScriptCode>(GetResources().CreateResourceHandleInternal(CreatePtrInternal(data, editorScript)));
}

SPtr<ScriptCode> ScriptCode::CreatePtrInternal(const WString& data, bool editorScript)
{
	SPtr<ScriptCode> scriptCodePtr = bs_core_ptr<ScriptCode>(
		new(bs_alloc<ScriptCode>()) ScriptCode(data, editorScript));
	scriptCodePtr->SetThisPtrInternal(scriptCodePtr);
	scriptCodePtr->Initialize();

	return scriptCodePtr;
}

RTTITypeBase* ScriptCode::GetRttiStatic()
{
	return ScriptCodeRTTI::Instance();
}

RTTITypeBase* ScriptCode::GetRtti() const
{
	return ScriptCode::GetRttiStatic();
}
