//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace b3d
{
	struct __ContactPointInterop
	{
		TVector3<float> Position;
		TVector3<float> Normal;
		float Impulse;
		float Separation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptContactPoint : public TScriptTypeDefinition<ScriptContactPoint>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ContactPoint")

		static MonoObject* Box(const __ContactPointInterop& value);
		static __ContactPointInterop Unbox(MonoObject* value);
		static ContactPoint FromInterop(const __ContactPointInterop& value);
		static __ContactPointInterop ToInterop(const ContactPoint& value);

	private:
		ScriptContactPoint();

	};
}
