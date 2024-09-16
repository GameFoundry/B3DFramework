//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTVector4.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVector4::ScriptVector4()
	{ }

	MonoObject* ScriptVector4::Box(const TVector4<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector4<float> ScriptVector4::Unbox(MonoObject* value)
	{
		return *(TVector4<float>*)MonoUtil::Unbox(value);
	}


	ScriptVector4d::ScriptVector4d()
	{ }

	MonoObject* ScriptVector4d::Box(const TVector4<double>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector4<double> ScriptVector4d::Unbox(MonoObject* value)
	{
		return *(TVector4<double>*)MonoUtil::Unbox(value);
	}

}
