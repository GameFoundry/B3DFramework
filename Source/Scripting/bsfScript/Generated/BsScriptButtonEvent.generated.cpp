//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptButtonEvent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptButtonEvent::ScriptButtonEvent()
	{ }

	MonoObject* ScriptButtonEvent::Box(const ButtonEvent& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	ButtonEvent ScriptButtonEvent::Unbox(MonoObject* value)
	{
		return *(ButtonEvent*)MonoUtil::Unbox(value);
	}

}
