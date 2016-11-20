//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptCode.h"
#include "BsResources.h"
#include "BsScriptCodeRTTI.h"

namespace bs
{
	ScriptCode::ScriptCode(const WString& data, bool editorScript)
		:Resource(false), mString(data), mEditorScript(editorScript)
	{

	}

	HScriptCode ScriptCode::create(const WString& data, bool editorScript)
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
		return ScriptCodeRTTI::instance();
	}

	RTTITypeBase* ScriptCode::getRTTI() const
	{
		return ScriptCode::getRTTIStatic();
	}
}