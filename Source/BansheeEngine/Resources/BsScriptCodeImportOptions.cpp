//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Resources/BsScriptCodeImportOptions.h"
#include "Private/RTTI/BsScriptCodeImportOptionsRTTI.h"

namespace bs
{
	ScriptCodeImportOptions::ScriptCodeImportOptions()
		:mEditorScript(false)
	{

	}

	SPtr<ScriptCodeImportOptions> ScriptCodeImportOptions::create()
	{
		return bs_shared_ptr_new<ScriptCodeImportOptions>();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* ScriptCodeImportOptions::getRTTIStatic()
	{
		return ScriptCodeImportOptionsRTTI::instance();
	}

	RTTITypeBase* ScriptCodeImportOptions::getRTTI() const
	{
		return ScriptCodeImportOptions::getRTTIStatic();
	}
}