//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsAABox.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsAABox.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace b3d
{
	struct __TAABox_float_Interop
	{
		TVector3<float> Minimum;
		TVector3<float> Maximum;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptAABox : public TScriptTypeDefinition<ScriptAABox>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AABox")

		static MonoObject* Box(const __TAABox_float_Interop& value);
		static __TAABox_float_Interop Unbox(MonoObject* value);
		static TAABox<float> FromInterop(const __TAABox_float_Interop& value);
		static __TAABox_float_Interop ToInterop(const TAABox<float>& value);

	private:
		ScriptAABox();

	};

	struct __TAABox_double_Interop
	{
		TVector3<double> Minimum;
		TVector3<double> Maximum;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptAABoxD : public TScriptTypeDefinition<ScriptAABoxD>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AABoxD")

		static MonoObject* Box(const __TAABox_double_Interop& value);
		static __TAABox_double_Interop Unbox(MonoObject* value);
		static TAABox<double> FromInterop(const __TAABox_double_Interop& value);
		static __TAABox_double_Interop ToInterop(const TAABox<double>& value);

	private:
		ScriptAABoxD();

	};
}
