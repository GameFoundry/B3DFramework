//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSpriteGlyphCreateInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Text/B3DFont.h"
#include "B3DScriptFont.generated.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptSpriteSheetGridAnimation.generated.h"

namespace b3d
{
	ScriptSpriteGlyphCreateInformation::ScriptSpriteGlyphCreateInformation()
	{ }

	MonoObject* ScriptSpriteGlyphCreateInformation::Box(const __SpriteGlyphCreateInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__SpriteGlyphCreateInformationInterop ScriptSpriteGlyphCreateInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteGlyphCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteGlyphCreateInformation ScriptSpriteGlyphCreateInformation::FromInterop(const __SpriteGlyphCreateInformationInterop& value)
	{
		SpriteGlyphCreateInformation output;
		TResourceHandle<Font> tmpFont;
		ScriptRRefBase* scriptObjectWrapperFont;
		scriptObjectWrapperFont = ScriptRRefBase::GetScriptObjectWrapper(value.Font);
		if(scriptObjectWrapperFont != nullptr)
			tmpFont = B3DStaticResourceCast<Font>(scriptObjectWrapperFont->GetNativeObject());
		output.Font = tmpFont;
		output.Glyph = value.Glyph;
		output.DefaultSize = value.DefaultSize;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteGlyphCreateInformationInterop ScriptSpriteGlyphCreateInformation::ToInterop(const SpriteGlyphCreateInformation& value)
	{
		__SpriteGlyphCreateInformationInterop output;
		MonoObject* tmpFont;
		ScriptRRefBase* scriptWrapperObjectFont;
		scriptWrapperObjectFont = ScriptResourceManager::Instance().GetScriptRRef(value.Font);
		if(scriptWrapperObjectFont != nullptr)
			tmpFont = scriptWrapperObjectFont->GetScriptObject();
		else
			tmpFont = nullptr;
		output.Font = tmpFont;
		output.Glyph = value.Glyph;
		output.DefaultSize = value.DefaultSize;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
