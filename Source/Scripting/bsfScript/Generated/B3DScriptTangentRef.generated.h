//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Animation/B3DAnimationUtility.h"
#include "../../../Foundation/Core/Animation/B3DAnimationUtility.h"
#include "../../../Foundation/Core/Animation/B3DAnimationUtility.h"

namespace b3d
{
	struct __TangentRefInterop
	{
		KeyframeRef KeyframeRef;
		TangentType Type;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTangentRef : public TScriptTypeDefinition<ScriptTangentRef>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TangentRef")

		static MonoObject* Box(const __TangentRefInterop& value);
		static __TangentRefInterop Unbox(MonoObject* value);
		static TangentRef FromInterop(const __TangentRefInterop& value);
		static __TangentRefInterop ToInterop(const TangentRef& value);

	private:
		ScriptTangentRef();

	};
}
