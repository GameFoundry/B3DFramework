//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Components/BsD6Joint.h"

namespace b3d
{
	struct __D6JointDriveInterop
	{
		float Stiffness;
		float Damping;
		float ForceLimit;
		bool Acceleration;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptD6JointDrive : public TScriptTypeDefinition<ScriptD6JointDrive>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "D6JointDrive")

		static MonoObject* Box(const __D6JointDriveInterop& value);
		static __D6JointDriveInterop Unbox(MonoObject* value);
		static D6JointDrive FromInterop(const __D6JointDriveInterop& value);
		static __D6JointDriveInterop ToInterop(const D6JointDrive& value);

	private:
		ScriptD6JointDrive();

	};
}
