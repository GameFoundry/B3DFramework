//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptFontCreateInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptFontCreateInformation::ScriptFontCreateInformation()
	{ }

	MonoObject* ScriptFontCreateInformation::Box(const __FontCreateInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__FontCreateInformationInterop ScriptFontCreateInformation::Unbox(MonoObject* value)
	{
		return *(__FontCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	FontCreateInformation ScriptFontCreateInformation::FromInterop(const __FontCreateInformationInterop& value)
	{
		FontCreateInformation output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.DPI = value.DPI;
		output.RenderMode = value.RenderMode;

		return output;
	}

	__FontCreateInformationInterop ScriptFontCreateInformation::ToInterop(const FontCreateInformation& value)
	{
		__FontCreateInformationInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.DPI = value.DPI;
		output.RenderMode = value.RenderMode;

		return output;
	}

}
