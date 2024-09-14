//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshData.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoneWeight : public TScriptStructWrapper<ScriptBoneWeight>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BoneWeight")

		static MonoObject* Box(const BoneWeight& value);
		static BoneWeight Unbox(MonoObject* value);

	private:
		ScriptBoneWeight();

	};
}
