//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsRay.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsRay.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace b3d
{
	struct __TRay_float_Interop
	{
		TVector3<float> Origin;
		TVector3<float> Direction;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptRay : public TScriptTypeDefinition<ScriptRay>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Ray")

		static MonoObject* Box(const __TRay_float_Interop& value);
		static __TRay_float_Interop Unbox(MonoObject* value);
		static TRay<float> FromInterop(const __TRay_float_Interop& value);
		static __TRay_float_Interop ToInterop(const TRay<float>& value);

	private:
		ScriptRay();

	};

	struct __TRay_double_Interop
	{
		TVector3<double> Origin;
		TVector3<double> Direction;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptRayD : public TScriptTypeDefinition<ScriptRayD>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RayD")

		static MonoObject* Box(const __TRay_double_Interop& value);
		static __TRay_double_Interop Unbox(MonoObject* value);
		static TRay<double> FromInterop(const __TRay_double_Interop& value);
		static __TRay_double_Interop ToInterop(const TRay<double>& value);

	private:
		ScriptRayD();

	};
}
