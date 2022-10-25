//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptContactPoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptContactPoint::ScriptContactPoint(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptContactPoint::InitRuntimeData()
	{}

	MonoObject* ScriptContactPoint::Box(const __ContactPointInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ContactPointInterop ScriptContactPoint::Unbox(MonoObject* value)
	{
		return *(__ContactPointInterop*)MonoUtil::Unbox(value);
	}

	ContactPoint ScriptContactPoint::FromInterop(const __ContactPointInterop& value)
	{
		ContactPoint output;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.Impulse = value.Impulse;
		output.Separation = value.Separation;

		return output;
	}

	__ContactPointInterop ScriptContactPoint::ToInterop(const ContactPoint& value)
	{
		__ContactPointInterop output;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.Impulse = value.Impulse;
		output.Separation = value.Separation;

		return output;
	}

} // namespace bs
