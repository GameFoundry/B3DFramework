//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Physics/B3DColliderShape.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d
{
	struct __BoxColliderShapeInformationInterop
	{
		TVector3<float> Extents;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoxColliderShapeInformation : public TScriptTypeDefinition<ScriptBoxColliderShapeInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BoxColliderShapeInformation")

		static MonoObject* Box(const __BoxColliderShapeInformationInterop& value);
		static __BoxColliderShapeInformationInterop Unbox(MonoObject* value);
		static BoxColliderShapeInformation FromInterop(const __BoxColliderShapeInformationInterop& value);
		static __BoxColliderShapeInformationInterop ToInterop(const BoxColliderShapeInformation& value);

	private:
		ScriptBoxColliderShapeInformation();

	};
}
