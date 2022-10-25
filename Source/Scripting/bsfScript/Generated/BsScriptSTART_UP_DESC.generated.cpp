//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSTART_UP_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderWindow.h"
#include "BsScriptRENDER_WINDOW_DESC.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptSTART_UP_DESC::ScriptSTART_UP_DESC(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptSTART_UP_DESC::InitRuntimeData()
	{}

	MonoObject* ScriptSTART_UP_DESC::Box(const __START_UP_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__START_UP_DESCInterop ScriptSTART_UP_DESC::Unbox(MonoObject* value)
	{
		return *(__START_UP_DESCInterop*)MonoUtil::Unbox(value);
	}

	START_UP_DESC ScriptSTART_UP_DESC::FromInterop(const __START_UP_DESCInterop& value)
	{
		START_UP_DESC output;
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
		RENDER_WINDOW_DESC tmpPrimaryWindowDesc;
		tmpPrimaryWindowDesc = ScriptRENDER_WINDOW_DESC::FromInterop(value.PrimaryWindowDesc);
		output.PrimaryWindowDesc = tmpPrimaryWindowDesc;
		Vector<String> vecImporters;
		if(value.Importers != nullptr)
		{
			ScriptArray arrayImporters(value.Importers);
			vecImporters.resize(arrayImporters.Size());
			for(int i = 0; i < (int)arrayImporters.Size(); i++)
			{
				vecImporters[i] = arrayImporters.Get<String>(i);
			}
		}
		output.Importers = vecImporters;

		return output;
	}

	__START_UP_DESCInterop ScriptSTART_UP_DESC::ToInterop(const START_UP_DESC& value)
	{
		__START_UP_DESCInterop output;
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
		__RENDER_WINDOW_DESCInterop tmpPrimaryWindowDesc;
		tmpPrimaryWindowDesc = ScriptRENDER_WINDOW_DESC::ToInterop(value.PrimaryWindowDesc);
		output.PrimaryWindowDesc = tmpPrimaryWindowDesc;
		int arraySizeImporters = (int)value.Importers.size();
		MonoArray* vecImporters;
		ScriptArray arrayImporters = ScriptArray::Create<String>(arraySizeImporters);
		for(int i = 0; i < arraySizeImporters; i++)
		{
			arrayImporters.Set(i, value.Importers[i]);
		}
		vecImporters = arrayImporters.GetInternal();
		output.Importers = vecImporters;

		return output;
	}

#endif
} // namespace bs
