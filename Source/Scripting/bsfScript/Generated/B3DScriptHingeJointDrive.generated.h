//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DHingeJoint.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptHingeJointDrive : public TScriptTypeDefinition<ScriptHingeJointDrive>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "HingeJointDrive")

		static MonoObject* Box(const HingeJointDrive& value);
		static HingeJointDrive Unbox(MonoObject* value);

	private:
		ScriptHingeJointDrive();

	};
}
