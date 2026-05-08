//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptManagedMemberInfo.generated.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedFieldInfo; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedFieldInfo : public TScriptReflectableWrapper<ManagedFieldInfo, ScriptManagedFieldInfo, ScriptManagedMemberInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedFieldInfo")

		ScriptManagedFieldInfo(const TShared<ManagedFieldInfo>& nativeObject);
		~ScriptManagedFieldInfo();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
