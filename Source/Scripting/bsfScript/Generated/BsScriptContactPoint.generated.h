//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __ContactPointInterop
	{
		TVector3<float> Position;
		TVector3<float> Normal;
		float Impulse;
		float Separation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptContactPoint : public ScriptObject<ScriptContactPoint>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ContactPoint")

		static MonoObject* Box(const __ContactPointInterop& value);
		static __ContactPointInterop Unbox(MonoObject* value);
		static ContactPoint FromInterop(const __ContactPointInterop& value);
		static __ContactPointInterop ToInterop(const ContactPoint& value);

	private:
		ScriptContactPoint(MonoObject* managedInstance);

	};
}
