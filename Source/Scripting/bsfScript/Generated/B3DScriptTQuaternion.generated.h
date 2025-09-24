//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DQuaternion.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DQuaternion.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptQuaternion : public TScriptTypeDefinition<ScriptQuaternion>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Quaternion")

		static MonoObject* Box(const TQuaternion<float>& value);
		static TQuaternion<float> Unbox(MonoObject* value);

	private:
		ScriptQuaternion();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptQuaternionD : public TScriptTypeDefinition<ScriptQuaternionD>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "QuaternionD")

		static MonoObject* Box(const TQuaternion<double>& value);
		static TQuaternion<double> Unbox(MonoObject* value);

	private:
		ScriptQuaternionD();

	};
}
