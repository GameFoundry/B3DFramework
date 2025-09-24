//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsRect3.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace b3d
{
	struct __Rect3Interop
	{
		TVector3<float> Center;
		TVector3<float> HorizontalAxis;
		TVector3<float> VerticalAxis;
		float HorizontalExtent;
		float VerticalExtent;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptRect3 : public TScriptTypeDefinition<ScriptRect3>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Rect3")

		static MonoObject* Box(const __Rect3Interop& value);
		static __Rect3Interop Unbox(MonoObject* value);
		static Rect3 FromInterop(const __Rect3Interop& value);
		static __Rect3Interop ToInterop(const Rect3& value);

	private:
		ScriptRect3();

	};
}
