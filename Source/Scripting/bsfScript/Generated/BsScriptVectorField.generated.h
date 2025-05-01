//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace bs { class VectorField; }
namespace bs { struct __VECTOR_FIELD_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorField : public TScriptResourceWrapper<VectorField, ScriptVectorField>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VectorField")

		ScriptVectorField(const TResourceHandle<VectorField>& nativeObject);
		~ScriptVectorField();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptVectorField* self);

		static void InternalCreate(MonoObject* scriptObject, __VECTOR_FIELD_DESCInterop* desc, MonoArray* values);
	};
}
