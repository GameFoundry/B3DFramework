//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3 : public TScriptTypeDefinition<ScriptVector3>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector3")

		static MonoObject* Box(const TVector3<float>& value);
		static TVector3<float> Unbox(MonoObject* value);

	private:
		ScriptVector3();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3D : public TScriptTypeDefinition<ScriptVector3D>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector3D")

		static MonoObject* Box(const TVector3<double>& value);
		static TVector3<double> Unbox(MonoObject* value);

	private:
		ScriptVector3D();

	};
}
