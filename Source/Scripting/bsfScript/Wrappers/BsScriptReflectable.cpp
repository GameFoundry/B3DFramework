//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptReflectable.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	ScriptReflectableBase::ScriptReflectableBase(MonoObject* instance)
		:ScriptObjectBase(instance)
	{ }
}
