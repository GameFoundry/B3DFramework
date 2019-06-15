//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBlend1DInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "BsScriptBlendClipInfo.generated.h"

namespace bs
{
	ScriptBlend1DInfo::ScriptBlend1DInfo(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptBlend1DInfo::initRuntimeData()
	{ }

	MonoObject*ScriptBlend1DInfo::box(const __Blend1DInfoInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__Blend1DInfoInterop ScriptBlend1DInfo::unbox(MonoObject* value)
	{
		return *(__Blend1DInfoInterop*)MonoUtil::unbox(value);
	}

	Blend1DInfo ScriptBlend1DInfo::fromInterop(const __Blend1DInfoInterop& value)
	{
		Blend1DInfo output;
		Vector<BlendClipInfo> vecclips;
		if(value.clips != nullptr)
		{
			ScriptArray arrayclips(value.clips);
			vecclips.resize(arrayclips.size());
			for(int i = 0; i < (int)arrayclips.size(); i++)
			{
				vecclips[i] = ScriptBlendClipInfo::fromInterop(arrayclips.get<__BlendClipInfoInterop>(i));
			}
		}
		output.clips = vecclips;

		return output;
	}

	__Blend1DInfoInterop ScriptBlend1DInfo::toInterop(const Blend1DInfo& value)
	{
		__Blend1DInfoInterop output;
		int arraySizeclips = (int)value.clips.size();
		MonoArray* vecclips;
		ScriptArray arrayclips = ScriptArray::create<ScriptBlendClipInfo>(arraySizeclips);
		for(int i = 0; i < arraySizeclips; i++)
		{
			arrayclips.set(i, ScriptBlendClipInfo::toInterop(value.clips[i]));
		}
		vecclips = arrayclips.getInternal();
		output.clips = vecclips;

		return output;
	}

}
