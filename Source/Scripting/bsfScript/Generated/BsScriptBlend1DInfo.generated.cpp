//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	void ScriptBlend1DInfo::InitRuntimeData()
	{ }

	MonoObject*ScriptBlend1DInfo::Box(const __Blend1DInfoInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__Blend1DInfoInterop ScriptBlend1DInfo::Unbox(MonoObject* value)
	{
		return *(__Blend1DInfoInterop*)MonoUtil::Unbox(value);
	}

	Blend1DInfo ScriptBlend1DInfo::FromInterop(const __Blend1DInfoInterop& value)
	{
		Blend1DInfo output;
		Vector<BlendClipInfo> vecClips;
		if(value.Clips != nullptr)
		{
			ScriptArray arrayClips(value.Clips);
			vecClips.resize(arrayClips.Size());
			for(int i = 0; i < (int)arrayClips.Size(); i++)
			{
				vecClips[i] = ScriptBlendClipInfo::FromInterop(arrayClips.Get<__BlendClipInfoInterop>(i));
			}
		}
		output.Clips = vecClips;

		return output;
	}

	__Blend1DInfoInterop ScriptBlend1DInfo::ToInterop(const Blend1DInfo& value)
	{
		__Blend1DInfoInterop output;
		int arraySizeClips = (int)value.Clips.size();
		MonoArray* vecClips;
		ScriptArray arrayClips = ScriptArray::Create<ScriptBlendClipInfo>(arraySizeClips);
		for(int i = 0; i < arraySizeClips; i++)
		{
			arrayClips.Set(i, ScriptBlendClipInfo::ToInterop(value.Clips[i]));
		}
		vecClips = arrayClips.GetInternal();
		output.Clips = vecClips;

		return output;
	}

}
