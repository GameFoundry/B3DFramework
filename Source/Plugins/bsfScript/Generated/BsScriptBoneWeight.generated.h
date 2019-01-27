//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Mesh/BsMeshData.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptBoneWeight : public ScriptObject<ScriptBoneWeight>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "BoneWeight")

		static MonoObject* box(const BoneWeight& value);
		static BoneWeight unbox(MonoObject* value);

	private:
		ScriptBoneWeight(MonoObject* managedInstance);

	};
}
