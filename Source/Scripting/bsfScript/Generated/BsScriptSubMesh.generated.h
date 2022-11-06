//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsSubMesh.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSubMesh : public ScriptObject<ScriptSubMesh>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SubMesh")

		static MonoObject* Box(const SubMesh& value);
		static SubMesh Unbox(MonoObject* value);

	private:
		ScriptSubMesh(MonoObject* managedInstance);

	};
}
