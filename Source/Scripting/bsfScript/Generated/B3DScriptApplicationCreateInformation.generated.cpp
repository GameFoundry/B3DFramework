//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptApplicationCreateInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/RenderAPI/B3DRenderWindow.h"
#include "B3DScriptRenderWindowCreateInformation.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptApplicationCreateInformation::ScriptApplicationCreateInformation()
	{ }

	MonoObject* ScriptApplicationCreateInformation::Box(const __ApplicationCreateInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ApplicationCreateInformationInterop ScriptApplicationCreateInformation::Unbox(MonoObject* value)
	{
		return *(__ApplicationCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	ApplicationCreateInformation ScriptApplicationCreateInformation::FromInterop(const __ApplicationCreateInformationInterop& value)
	{
		ApplicationCreateInformation output;
		String tmpRenderApi;
		tmpRenderApi = MonoUtil::MonoToString(value.RenderApi);
		output.RenderApi = tmpRenderApi;
		String tmpRenderer;
		tmpRenderer = MonoUtil::MonoToString(value.Renderer);
		output.Renderer = tmpRenderer;
		String tmpPhysics;
		tmpPhysics = MonoUtil::MonoToString(value.Physics);
		output.Physics = tmpPhysics;
		String tmpAudio;
		tmpAudio = MonoUtil::MonoToString(value.Audio);
		output.Audio = tmpAudio;
		String tmpInput;
		tmpInput = MonoUtil::MonoToString(value.Input);
		output.Input = tmpInput;
		output.PhysicsCooking = value.PhysicsCooking;
		output.AsyncAnimation = value.AsyncAnimation;
		RenderWindowCreateInformation tmpPrimaryWindow;
		tmpPrimaryWindow = ScriptRenderWindowCreateInformation::FromInterop(value.PrimaryWindow);
		output.PrimaryWindow = tmpPrimaryWindow;
		Vector<String> vecImporters;
		if(value.Importers != nullptr)
		{
			ScriptArray scriptArrayImporters(value.Importers);
			vecImporters.resize(scriptArrayImporters.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayImporters.Size(); elementIndex++)
			{
				vecImporters[elementIndex] = scriptArrayImporters.Get<String>(elementIndex);
			}
		}
		output.Importers = vecImporters;

		return output;
	}

	__ApplicationCreateInformationInterop ScriptApplicationCreateInformation::ToInterop(const ApplicationCreateInformation& value)
	{
		__ApplicationCreateInformationInterop output;
		MonoString* tmpRenderApi;
		tmpRenderApi = MonoUtil::StringToMono(value.RenderApi);
		output.RenderApi = tmpRenderApi;
		MonoString* tmpRenderer;
		tmpRenderer = MonoUtil::StringToMono(value.Renderer);
		output.Renderer = tmpRenderer;
		MonoString* tmpPhysics;
		tmpPhysics = MonoUtil::StringToMono(value.Physics);
		output.Physics = tmpPhysics;
		MonoString* tmpAudio;
		tmpAudio = MonoUtil::StringToMono(value.Audio);
		output.Audio = tmpAudio;
		MonoString* tmpInput;
		tmpInput = MonoUtil::StringToMono(value.Input);
		output.Input = tmpInput;
		output.PhysicsCooking = value.PhysicsCooking;
		output.AsyncAnimation = value.AsyncAnimation;
		__RenderWindowCreateInformationInterop tmpPrimaryWindow;
		tmpPrimaryWindow = ScriptRenderWindowCreateInformation::ToInterop(value.PrimaryWindow);
		output.PrimaryWindow = tmpPrimaryWindow;
		int elementCountImporters = (int)value.Importers.size();
		MonoArray* vecImporters;
		ScriptArray scriptArrayImporters = ScriptArray::Create<String>(elementCountImporters);
		for(int elementIndex = 0; elementIndex < elementCountImporters; elementIndex++)
		{
			scriptArrayImporters.Set(elementIndex, value.Importers[elementIndex]);
		}
		vecImporters = scriptArrayImporters.GetInternal();
		output.Importers = vecImporters;

		return output;
	}

#endif
}
