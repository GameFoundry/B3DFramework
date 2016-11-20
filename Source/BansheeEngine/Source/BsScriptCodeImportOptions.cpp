//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptCodeImportOptions.h"
#include "BsScriptCodeImportOptionsRTTI.h"

namespace bs
{
	ScriptCodeImportOptions::ScriptCodeImportOptions()
		:mEditorScript(false)
	{

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