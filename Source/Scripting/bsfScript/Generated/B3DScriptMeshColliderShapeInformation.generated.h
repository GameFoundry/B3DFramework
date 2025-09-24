//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Physics/B3DColliderShape.h"

namespace b3d
{
	struct __MeshColliderShapeInformationInterop
	{
		MonoObject* Mesh;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptMeshColliderShapeInformation : public TScriptTypeDefinition<ScriptMeshColliderShapeInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MeshColliderShapeInformation")

		static MonoObject* Box(const __MeshColliderShapeInformationInterop& value);
		static __MeshColliderShapeInformationInterop Unbox(MonoObject* value);
		static MeshColliderShapeInformation FromInterop(const __MeshColliderShapeInformationInterop& value);
		static __MeshColliderShapeInformationInterop ToInterop(const MeshColliderShapeInformation& value);

	private:
		ScriptMeshColliderShapeInformation();

	};
}
