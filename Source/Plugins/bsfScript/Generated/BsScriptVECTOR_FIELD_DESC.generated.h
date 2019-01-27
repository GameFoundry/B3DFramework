//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "Math/BsAABox.h"

namespace bs
{
	struct __VECTOR_FIELD_DESCInterop
	{
		uint32_t countX;
		uint32_t countY;
		uint32_t countZ;
		AABox bounds;
	};

	class BS_SCR_BE_EXPORT ScriptVECTOR_FIELD_DESC : public ScriptObject<ScriptVECTOR_FIELD_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "VectorFieldOptions")

		static MonoObject* box(const __VECTOR_FIELD_DESCInterop& value);
		static __VECTOR_FIELD_DESCInterop unbox(MonoObject* value);
		static VECTOR_FIELD_DESC fromInterop(const __VECTOR_FIELD_DESCInterop& value);
		static __VECTOR_FIELD_DESCInterop toInterop(const VECTOR_FIELD_DESC& value);

	private:
		ScriptVECTOR_FIELD_DESC(MonoObject* managedInstance);

	};
}
