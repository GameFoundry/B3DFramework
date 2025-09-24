//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptContactPoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

namespace b3d
{
	ScriptContactPoint::ScriptContactPoint()
	{ }

	MonoObject* ScriptContactPoint::Box(const __ContactPointInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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

}
