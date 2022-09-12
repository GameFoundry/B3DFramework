//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsScriptCode.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsScriptCodeRTTI.h"

namespace bs
{
	ScriptCode::ScriptCode(const WString& data, bool editorScript)
		:Resource(false), mString(data), mEditorScript(editorScript)
	{

	}

	HScriptCode ScriptCode::Create(const WString& data, bool editorScript)
	{
		return static_resource_cast<ScriptCode>(gResources()._createResourceHandle(_createPtr(data, editorScript)));
	}

	SPtr<ScriptCode> ScriptCode::_createPtr(const WString& data, bool editorScript)
	{
		SPtr<ScriptCode> scriptCodePtr = bs_core_ptr<ScriptCode>(
			new (bs_alloc<ScriptCode>()) ScriptCode(data, editorScript));
		scriptCodePtr->_setThisPtr(scriptCodePtr);
		scriptCodePtr->initialize();

		return scriptCodePtr;
	}

	RTTITypeBase* ScriptCode::getRTTIStatic()
	{
		return ScriptCodeRTTI::Instance();
	}

	RTTITypeBase* ScriptCode::getRTTI() const
	{
		return ScriptCode::GetRTTIStatic();
	}
}
