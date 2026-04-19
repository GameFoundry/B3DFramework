//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DScriptCode.h"
#include "Resources/B3DResources.h"
#include "RTTI/B3DScriptCodeRTTI.h"

using namespace b3d;

ScriptCode::ScriptCode(const WString& data, bool editorScript)
	: Resource(false), mString(data), mEditorScript(editorScript)
{
}

HScriptCode ScriptCode::Create(const WString& data, bool editorScript)
{
	return B3DStaticResourceCast<ScriptCode>(GetResources().CreateResourceHandle(CreatePtrInternal(data, editorScript)));
}

SPtr<ScriptCode> ScriptCode::CreatePtrInternal(const WString& data, bool editorScript)
{
	SPtr<ScriptCode> scriptCodePtr = B3DMakeSharedFromExisting<ScriptCode>(
		new(B3DAllocate<ScriptCode>()) ScriptCode(data, editorScript));
	scriptCodePtr->SetShared(scriptCodePtr);
	scriptCodePtr->Initialize();

	return scriptCodePtr;
}

RTTIType* ScriptCode::GetRttiStatic()
{
	return ScriptCodeRTTI::Instance();
}

RTTIType* ScriptCode::GetRtti() const
{
	return ScriptCode::GetRttiStatic();
}
