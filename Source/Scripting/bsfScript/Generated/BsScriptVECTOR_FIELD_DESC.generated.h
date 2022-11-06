//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "Math/BsAABox.h"

namespace bs
{
	struct __VECTOR_FIELD_DESCInterop
	{
		uint32_t CountX;
		uint32_t CountY;
		uint32_t CountZ;
		AABox Bounds;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVECTOR_FIELD_DESC : public ScriptObject<ScriptVECTOR_FIELD_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "VectorFieldOptions")

		static MonoObject* Box(const __VECTOR_FIELD_DESCInterop& value);
		static __VECTOR_FIELD_DESCInterop Unbox(MonoObject* value);
		static VECTOR_FIELD_DESC FromInterop(const __VECTOR_FIELD_DESCInterop& value);
		static __VECTOR_FIELD_DESCInterop ToInterop(const VECTOR_FIELD_DESC& value);

	private:
		ScriptVECTOR_FIELD_DESC(MonoObject* managedInstance);

	};
}
