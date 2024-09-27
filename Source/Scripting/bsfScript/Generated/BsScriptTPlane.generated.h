//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsPlane.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsPlane.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace bs
{
	struct __TPlane_float_Interop
	{
		TVector3<float> Normal;
		float D;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlane : public TScriptTypeDefinition<ScriptPlane>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Plane")

		static MonoObject* Box(const __TPlane_float_Interop& value);
		static __TPlane_float_Interop Unbox(MonoObject* value);
		static TPlane<float> FromInterop(const __TPlane_float_Interop& value);
		static __TPlane_float_Interop ToInterop(const TPlane<float>& value);

	private:
		ScriptPlane();

	};

	struct __TPlane_double_Interop
	{
		TVector3<double> Normal;
		double D;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlaneD : public TScriptTypeDefinition<ScriptPlaneD>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PlaneD")

		static MonoObject* Box(const __TPlane_double_Interop& value);
		static __TPlane_double_Interop Unbox(MonoObject* value);
		static TPlane<double> FromInterop(const __TPlane_double_Interop& value);
		static __TPlane_double_Interop ToInterop(const TPlane<double>& value);

	private:
		ScriptPlaneD();

	};
}
