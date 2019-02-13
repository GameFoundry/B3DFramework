//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsScriptCodeImportOptions.h"
#include "Private/RTTI/BsScriptCodeImportOptionsRTTI.h"

namespace bs
{
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