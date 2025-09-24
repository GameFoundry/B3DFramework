//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "../../../Foundation/bsfUtility/Math/BsAABox.h"
#include "BsScriptTAABox.generated.h"

namespace b3d
{
	struct __VECTOR_FIELD_DESCInterop
	{
		uint32_t CountX;
		uint32_t CountY;
		uint32_t CountZ;
		__TAABox_float_Interop Bounds;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorFieldOptions : public TScriptTypeDefinition<ScriptVectorFieldOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VectorFieldOptions")

		static MonoObject* Box(const __VECTOR_FIELD_DESCInterop& value);
		static __VECTOR_FIELD_DESCInterop Unbox(MonoObject* value);
		static VECTOR_FIELD_DESC FromInterop(const __VECTOR_FIELD_DESCInterop& value);
		static __VECTOR_FIELD_DESCInterop ToInterop(const VECTOR_FIELD_DESC& value);

	private:
		ScriptVectorFieldOptions();

	};
}
