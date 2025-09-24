//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Physics/BsColliderShape.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlaneColliderShapeInformation : public TScriptTypeDefinition<ScriptPlaneColliderShapeInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PlaneColliderShapeInformation")

		static MonoObject* Box(const PlaneColliderShapeInformation& value);
		static PlaneColliderShapeInformation Unbox(MonoObject* value);

	private:
		ScriptPlaneColliderShapeInformation();

	};
}
