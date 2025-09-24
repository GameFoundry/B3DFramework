//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Mesh/B3DMeshData.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoneWeight : public TScriptTypeDefinition<ScriptBoneWeight>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BoneWeight")

		static MonoObject* Box(const BoneWeight& value);
		static BoneWeight Unbox(MonoObject* value);

	private:
		ScriptBoneWeight();

	};
}
