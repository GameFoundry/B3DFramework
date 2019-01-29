//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsSubMesh.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptSubMesh : public ScriptObject<ScriptSubMesh>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SubMesh")

		static MonoObject* box(const SubMesh& value);
		static SubMesh unbox(MonoObject* value);

	private:
		ScriptSubMesh(MonoObject* managedInstance);

	};
}
