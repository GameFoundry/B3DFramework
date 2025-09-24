//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Physics/B3DColliderShape.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCapsuleColliderShapeInformation : public TScriptTypeDefinition<ScriptCapsuleColliderShapeInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CapsuleColliderShapeInformation")

		static MonoObject* Box(const CapsuleColliderShapeInformation& value);
		static CapsuleColliderShapeInformation Unbox(MonoObject* value);

	private:
		ScriptCapsuleColliderShapeInformation();

	};
}
