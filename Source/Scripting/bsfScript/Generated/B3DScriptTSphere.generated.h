//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DSphere.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DSphere.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d
{
	struct __TSphere_float_Interop
	{
		float Radius;
		TVector3<float> Center;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphere : public TScriptTypeDefinition<ScriptSphere>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Sphere")

		static MonoObject* Box(const __TSphere_float_Interop& value);
		static __TSphere_float_Interop Unbox(MonoObject* value);
		static TSphere<float> FromInterop(const __TSphere_float_Interop& value);
		static __TSphere_float_Interop ToInterop(const TSphere<float>& value);

	private:
		ScriptSphere();

	};

	struct __TSphere_double_Interop
	{
		double Radius;
		TVector3<double> Center;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphereD : public TScriptTypeDefinition<ScriptSphereD>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SphereD")

		static MonoObject* Box(const __TSphere_double_Interop& value);
		static __TSphere_double_Interop Unbox(MonoObject* value);
		static TSphere<double> FromInterop(const __TSphere_double_Interop& value);
		static __TSphere_double_Interop ToInterop(const TSphere<double>& value);

	private:
		ScriptSphereD();

	};
}
