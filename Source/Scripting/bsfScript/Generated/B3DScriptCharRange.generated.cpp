//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptCharRange.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptCharRange::ScriptCharRange()
	{ }

	MonoObject* ScriptCharRange::Box(const CharRange& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	CharRange ScriptCharRange::Unbox(MonoObject* value)
	{
		return *(CharRange*)MonoUtil::Unbox(value);
	}

#endif
}
