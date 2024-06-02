//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "Math/BsVector3.h"

namespace bs { class VectorField; }
namespace bs { struct __VECTOR_FIELD_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorField : public TScriptResource<ScriptVectorField, VectorField>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "VectorField")

		ScriptVectorField(MonoObject* managedInstance, const TResourceHandle<VectorField>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptVectorField* thisPtr);

		static void InternalCreate(MonoObject* managedInstance, __VECTOR_FIELD_DESCInterop* desc, MonoArray* values);
	};
}
