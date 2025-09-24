//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTVector3I.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptVector3I::ScriptVector3I()
	{ }

	MonoObject* ScriptVector3I::Box(const TVector3I<int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector3I<int32_t> ScriptVector3I::Unbox(MonoObject* value)
	{
		return *(TVector3I<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptVector3UI::ScriptVector3UI()
	{ }

	MonoObject* ScriptVector3UI::Box(const TVector3I<uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector3I<uint32_t> ScriptVector3UI::Unbox(MonoObject* value)
	{
		return *(TVector3I<uint32_t>*)MonoUtil::Unbox(value);
	}

}
