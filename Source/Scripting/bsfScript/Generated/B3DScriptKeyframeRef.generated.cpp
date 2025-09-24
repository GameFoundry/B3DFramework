//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptKeyframeRef.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptKeyframeRef::ScriptKeyframeRef()
	{ }

	MonoObject* ScriptKeyframeRef::Box(const KeyframeRef& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	KeyframeRef ScriptKeyframeRef::Unbox(MonoObject* value)
	{
		return *(KeyframeRef*)MonoUtil::Unbox(value);
	}

}
