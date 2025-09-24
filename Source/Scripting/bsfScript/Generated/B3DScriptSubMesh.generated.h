//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsSubMesh.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSubMesh : public TScriptTypeDefinition<ScriptSubMesh>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SubMesh")

		static MonoObject* Box(const SubMesh& value);
		static SubMesh Unbox(MonoObject* value);

	private:
		ScriptSubMesh();

	};
}
