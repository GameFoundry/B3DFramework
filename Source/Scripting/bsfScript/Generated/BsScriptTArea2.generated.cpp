//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTArea2.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTArea2_int32_t__int32_t_::ScriptTArea2_int32_t__int32_t_()
	{ }

	MonoObject* ScriptTArea2_int32_t__int32_t_::Box(const TArea2<int32_t, int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TArea2<int32_t, int32_t> ScriptTArea2_int32_t__int32_t_::Unbox(MonoObject* value)
	{
		return *(TArea2<int32_t, int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTArea2_int32_t__uint32_t_::ScriptTArea2_int32_t__uint32_t_()
	{ }

	MonoObject* ScriptTArea2_int32_t__uint32_t_::Box(const TArea2<int32_t, uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TArea2<int32_t, uint32_t> ScriptTArea2_int32_t__uint32_t_::Unbox(MonoObject* value)
	{
		return *(TArea2<int32_t, uint32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTArea2_float__float_::ScriptTArea2_float__float_()
	{ }

	MonoObject* ScriptTArea2_float__float_::Box(const TArea2<float, float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TArea2<float, float> ScriptTArea2_float__float_::Unbox(MonoObject* value)
	{
		return *(TArea2<float, float>*)MonoUtil::Unbox(value);
	}

}
