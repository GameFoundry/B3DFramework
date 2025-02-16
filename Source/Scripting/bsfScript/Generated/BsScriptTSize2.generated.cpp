//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTSize2.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTSize2_float_::ScriptTSize2_float_()
	{ }

	MonoObject* ScriptTSize2_float_::Box(const TSize2<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize2<float> ScriptTSize2_float_::Unbox(MonoObject* value)
	{
		return *(TSize2<float>*)MonoUtil::Unbox(value);
	}


	ScriptTSize2_uint32_t_::ScriptTSize2_uint32_t_()
	{ }

	MonoObject* ScriptTSize2_uint32_t_::Box(const TSize2<uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize2<uint32_t> ScriptTSize2_uint32_t_::Unbox(MonoObject* value)
	{
		return *(TSize2<uint32_t>*)MonoUtil::Unbox(value);
	}

}
