//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsQuaternion.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsQuaternion.h"

namespace bs
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
