//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTVector4I.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptVector4I::ScriptVector4I()
	{ }

	MonoObject* ScriptVector4I::Box(const TVector4I<int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector4I<int32_t> ScriptVector4I::Unbox(MonoObject* value)
	{
		return *(TVector4I<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptVector4UI::ScriptVector4UI()
	{ }

	MonoObject* ScriptVector4UI::Box(const TVector4I<uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector4I<uint32_t> ScriptVector4UI::Unbox(MonoObject* value)
	{
		return *(TVector4I<uint32_t>*)MonoUtil::Unbox(value);
	}

}
