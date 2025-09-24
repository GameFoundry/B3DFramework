//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBlend1DInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsAnimation.h"
#include "BsScriptBlendClipInfo.generated.h"

namespace b3d
{
	ScriptBlend1DInfo::ScriptBlend1DInfo()
	{ }

	MonoObject* ScriptBlend1DInfo::Box(const __Blend1DInfoInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
			ScriptArray scriptArrayClips(value.Clips);
			vecClips.resize(scriptArrayClips.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayClips.Size(); elementIndex++)
			{
				vecClips[elementIndex] = ScriptBlendClipInfo::FromInterop(scriptArrayClips.Get<__BlendClipInfoInterop>(elementIndex));
			}
		}
		output.Clips = vecClips;

		return output;
	}

	__Blend1DInfoInterop ScriptBlend1DInfo::ToInterop(const Blend1DInfo& value)
	{
		__Blend1DInfoInterop output;
		int elementCountClips = (int)value.Clips.size();
		MonoArray* vecClips;
		ScriptArray scriptArrayClips = ScriptArray::Create<ScriptBlendClipInfo>(elementCountClips);
		for(int elementIndex = 0; elementIndex < elementCountClips; elementIndex++)
		{
			scriptArrayClips.Set(elementIndex, ScriptBlendClipInfo::ToInterop(value.Clips[elementIndex]));
		}
		vecClips = scriptArrayClips.GetInternal();
		output.Clips = vecClips;

		return output;
	}

}
