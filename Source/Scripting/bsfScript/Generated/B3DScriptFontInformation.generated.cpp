//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFontInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptFontInformation::ScriptFontInformation()
	{ }

	MonoObject* ScriptFontInformation::Box(const __FontInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__FontInformationInterop ScriptFontInformation::Unbox(MonoObject* value)
	{
		return *(__FontInformationInterop*)MonoUtil::Unbox(value);
	}

	FontInformation ScriptFontInformation::FromInterop(const __FontInformationInterop& value)
	{
		FontInformation output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.DPI = value.DPI;
		output.RenderMode = value.RenderMode;

		return output;
	}

	__FontInformationInterop ScriptFontInformation::ToInterop(const FontInformation& value)
	{
		__FontInformationInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.DPI = value.DPI;
		output.RenderMode = value.RenderMode;

		return output;
	}

}
