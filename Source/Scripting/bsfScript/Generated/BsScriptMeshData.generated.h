//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshData.h"

namespace bs { class MeshData; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMeshData : public TScriptReflectableWrapper<MeshData, ScriptMeshData>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MeshData")

		ScriptMeshData(const SPtr<MeshData>& nativeObject);
		~ScriptMeshData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
