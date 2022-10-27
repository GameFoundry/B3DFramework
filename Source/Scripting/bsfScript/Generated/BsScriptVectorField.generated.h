//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"

namespace bs
{
	class VectorField;
}

namespace bs
{
	struct __VECTOR_FIELD_DESCInterop;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptVectorField : public TScriptResource<ScriptVectorField, VectorField>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "VectorField")

		ScriptVectorField(MonoObject* managedInstance, const ResourceHandle<VectorField>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptVectorField* thisPtr);

		static void InternalCreate(MonoObject* managedInstance, __VECTOR_FIELD_DESCInterop* desc, MonoArray* values);
	};
} // namespace bs
