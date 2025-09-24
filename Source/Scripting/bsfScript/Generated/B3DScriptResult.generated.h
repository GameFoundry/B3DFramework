//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Utility/BsResult.h"
#include "../../../Foundation/bsfUtility/Utility/BsResult.h"

namespace b3d
{
	struct __ResultInterop
	{
		ResultStatus Status;
		MonoString* ErrorMessage;
		MonoString* AdditionalErrorMessage;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptResult : public TScriptTypeDefinition<ScriptResult>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Result")

		static MonoObject* Box(const __ResultInterop& value);
		static __ResultInterop Unbox(MonoObject* value);
		static Result FromInterop(const __ResultInterop& value);
		static __ResultInterop ToInterop(const Result& value);

	private:
		ScriptResult();

	};
}
