//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTVector2.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVector2::ScriptVector2()
	{ }

	MonoObject* ScriptVector2::Box(const TVector2<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<float> ScriptVector2::Unbox(MonoObject* value)
	{
		return *(TVector2<float>*)MonoUtil::Unbox(value);
	}


	ScriptVector2D::ScriptVector2D()
	{ }

	MonoObject* ScriptVector2D::Box(const TVector2<double>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<double> ScriptVector2D::Unbox(MonoObject* value)
	{
		return *(TVector2<double>*)MonoUtil::Unbox(value);
	}


	ScriptVector2I::ScriptVector2I()
	{ }

	MonoObject* ScriptVector2I::Box(const TVector2<int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<int32_t> ScriptVector2I::Unbox(MonoObject* value)
	{
		return *(TVector2<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptVector2UI::ScriptVector2UI()
	{ }

	MonoObject* ScriptVector2UI::Box(const TVector2<uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<uint32_t> ScriptVector2UI::Unbox(MonoObject* value)
	{
		return *(TVector2<uint32_t>*)MonoUtil::Unbox(value);
	}

}
