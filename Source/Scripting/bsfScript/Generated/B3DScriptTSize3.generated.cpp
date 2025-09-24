//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTSize3.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptSize3::ScriptSize3()
	{ }

	MonoObject* ScriptSize3::Box(const TSize3<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize3<float> ScriptSize3::Unbox(MonoObject* value)
	{
		return *(TSize3<float>*)MonoUtil::Unbox(value);
	}


	ScriptSize3UI::ScriptSize3UI()
	{ }

	MonoObject* ScriptSize3UI::Box(const TSize3<uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize3<uint32_t> ScriptSize3UI::Unbox(MonoObject* value)
	{
		return *(TSize3<uint32_t>*)MonoUtil::Unbox(value);
	}

}
