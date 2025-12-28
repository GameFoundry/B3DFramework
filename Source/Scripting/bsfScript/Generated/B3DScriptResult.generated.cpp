//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptResult.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptResult::ScriptResult()
	{ }

	MonoObject* ScriptResult::Box(const __ResultInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ResultInterop ScriptResult::Unbox(MonoObject* value)
	{
		return *(__ResultInterop*)MonoUtil::Unbox(value);
	}

	Result ScriptResult::FromInterop(const __ResultInterop& value)
	{
		Result output;
		output.Status = value.Status;
		B3D_LOG(Error, LogScript, "const char* type cannot be assigned from scripting for field 'ErrorMessage'. This is not supported for this type.");
		String tmpAdditionalErrorMessage;
		tmpAdditionalErrorMessage = MonoUtil::MonoToString(value.AdditionalErrorMessage);
		output.AdditionalErrorMessage = tmpAdditionalErrorMessage;

		return output;
	}

	__ResultInterop ScriptResult::ToInterop(const Result& value)
	{
		__ResultInterop output;
		output.Status = value.Status;
		MonoString* tmpErrorMessage;
		tmpErrorMessage = MonoUtil::StringToMono(value.ErrorMessage);
		output.ErrorMessage = tmpErrorMessage;
		MonoString* tmpAdditionalErrorMessage;
		tmpAdditionalErrorMessage = MonoUtil::StringToMono(value.AdditionalErrorMessage);
		output.AdditionalErrorMessage = tmpAdditionalErrorMessage;

		return output;
	}

}
