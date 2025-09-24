//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSpriteVectorPathCreateInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/VectorGraphics/B3DVectorGraphics.h"
#include "B3DScriptVectorPath.generated.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptSpriteSheetGridAnimation.generated.h"
#include "../../../Foundation/Utility/Math/B3DSize2.h"
#include "B3DScriptTSize2.generated.h"

namespace b3d
{
	ScriptSpriteVectorPathCreateInformation::ScriptSpriteVectorPathCreateInformation()
	{ }

	MonoObject* ScriptSpriteVectorPathCreateInformation::Box(const __SpriteVectorPathCreateInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__SpriteVectorPathCreateInformationInterop ScriptSpriteVectorPathCreateInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteVectorPathCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteVectorPathCreateInformation ScriptSpriteVectorPathCreateInformation::FromInterop(const __SpriteVectorPathCreateInformationInterop& value)
	{
		SpriteVectorPathCreateInformation output;
		TResourceHandle<VectorPath> tmpVectorPath;
		ScriptRRefBase* scriptObjectWrapperVectorPath;
		scriptObjectWrapperVectorPath = ScriptRRefBase::GetScriptObjectWrapper(value.VectorPath);
		if(scriptObjectWrapperVectorPath != nullptr)
			tmpVectorPath = B3DStaticResourceCast<VectorPath>(scriptObjectWrapperVectorPath->GetNativeObject());
		output.VectorPath = tmpVectorPath;
		output.DefaultSize = value.DefaultSize;
		output.ScalingMode = value.ScalingMode;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteVectorPathCreateInformationInterop ScriptSpriteVectorPathCreateInformation::ToInterop(const SpriteVectorPathCreateInformation& value)
	{
		__SpriteVectorPathCreateInformationInterop output;
		MonoObject* tmpVectorPath;
		ScriptRRefBase* scriptWrapperObjectVectorPath;
		scriptWrapperObjectVectorPath = ScriptResourceManager::Instance().GetScriptRRef(value.VectorPath);
		if(scriptWrapperObjectVectorPath != nullptr)
			tmpVectorPath = scriptWrapperObjectVectorPath->GetScriptObject();
		else
			tmpVectorPath = nullptr;
		output.VectorPath = tmpVectorPath;
		output.DefaultSize = value.DefaultSize;
		output.ScalingMode = value.ScalingMode;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
