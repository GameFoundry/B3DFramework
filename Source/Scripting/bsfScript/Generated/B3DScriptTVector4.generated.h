//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector4.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector4.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector4 : public TScriptTypeDefinition<ScriptVector4>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector4")

		static MonoObject* Box(const TVector4<float>& value);
		static TVector4<float> Unbox(MonoObject* value);

	private:
		ScriptVector4();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector4D : public TScriptTypeDefinition<ScriptVector4D>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector4D")

		static MonoObject* Box(const TVector4<double>& value);
		static TVector4<double> Unbox(MonoObject* value);

	private:
		ScriptVector4D();

	};
}
