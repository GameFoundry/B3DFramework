//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTextInputEvent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTextInputEvent::ScriptTextInputEvent()
	{ }

	MonoObject* ScriptTextInputEvent::Box(const TextInputEvent& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TextInputEvent ScriptTextInputEvent::Unbox(MonoObject* value)
	{
		return *(TextInputEvent*)MonoUtil::Unbox(value);
	}

}
