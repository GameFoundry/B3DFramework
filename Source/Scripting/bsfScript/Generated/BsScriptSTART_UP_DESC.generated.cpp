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
		:ScriptObject(managedInstance)
	{ }

	void ScriptSTART_UP_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptSTART_UP_DESC::box(const __START_UP_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__START_UP_DESCInterop ScriptSTART_UP_DESC::unbox(MonoObject* value)
	{
		return *(__START_UP_DESCInterop*)MonoUtil::unbox(value);
	}

	START_UP_DESC ScriptSTART_UP_DESC::fromInterop(const __START_UP_DESCInterop& value)
	{
		START_UP_DESC output;
		String tmprenderAPI;
		tmprenderAPI = MonoUtil::monoToString(value.renderAPI);
		output.renderAPI = tmprenderAPI;
		String tmprenderer;
		tmprenderer = MonoUtil::monoToString(value.renderer);
		output.renderer = tmprenderer;
		String tmpphysics;
		tmpphysics = MonoUtil::monoToString(value.physics);
		output.physics = tmpphysics;
		String tmpaudio;
		tmpaudio = MonoUtil::monoToString(value.audio);
		output.audio = tmpaudio;
		String tmpinput;
		tmpinput = MonoUtil::monoToString(value.input);
		output.input = tmpinput;
		output.physicsCooking = value.physicsCooking;
		output.asyncAnimation = value.asyncAnimation;
		RENDER_WINDOW_DESC tmpprimaryWindowDesc;
		tmpprimaryWindowDesc = ScriptRENDER_WINDOW_DESC::fromInterop(value.primaryWindowDesc);
		output.primaryWindowDesc = tmpprimaryWindowDesc;
		Vector<String> vecimporters;
		if(value.importers != nullptr)
		{
			ScriptArray arrayimporters(value.importers);
			vecimporters.resize(arrayimporters.size());
			for(int i = 0; i < (int)arrayimporters.size(); i++)
			{
				vecimporters[i] = arrayimporters.get<String>(i);
			}
		}
		output.importers = vecimporters;

		return output;
	}

	__START_UP_DESCInterop ScriptSTART_UP_DESC::toInterop(const START_UP_DESC& value)
	{
		__START_UP_DESCInterop output;
		MonoString* tmprenderAPI;
		tmprenderAPI = MonoUtil::stringToMono(value.renderAPI);
		output.renderAPI = tmprenderAPI;
		MonoString* tmprenderer;
		tmprenderer = MonoUtil::stringToMono(value.renderer);
		output.renderer = tmprenderer;
		MonoString* tmpphysics;
		tmpphysics = MonoUtil::stringToMono(value.physics);
		output.physics = tmpphysics;
		MonoString* tmpaudio;
		tmpaudio = MonoUtil::stringToMono(value.audio);
		output.audio = tmpaudio;
		MonoString* tmpinput;
		tmpinput = MonoUtil::stringToMono(value.input);
		output.input = tmpinput;
		output.physicsCooking = value.physicsCooking;
		output.asyncAnimation = value.asyncAnimation;
		__RENDER_WINDOW_DESCInterop tmpprimaryWindowDesc;
		tmpprimaryWindowDesc = ScriptRENDER_WINDOW_DESC::toInterop(value.primaryWindowDesc);
		output.primaryWindowDesc = tmpprimaryWindowDesc;
		int arraySizeimporters = (int)value.importers.size();
		MonoArray* vecimporters;
		ScriptArray arrayimporters = ScriptArray::create<String>(arraySizeimporters);
		for(int i = 0; i < arraySizeimporters; i++)
		{
			arrayimporters.set(i, value.importers[i]);
		}
		vecimporters = arrayimporters.getInternal();
		output.importers = vecimporters;

		return output;
	}

#endif
}
