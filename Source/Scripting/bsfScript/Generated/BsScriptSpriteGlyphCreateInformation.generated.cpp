//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteGlyphCreateInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "BsScriptFont.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptSpriteSheetGridAnimation.generated.h"

namespace bs
{
	ScriptSpriteGlyphCreateInformation::ScriptSpriteGlyphCreateInformation(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSpriteGlyphCreateInformation::InitRuntimeData()
	{ }

	MonoObject*ScriptSpriteGlyphCreateInformation::Box(const __SpriteGlyphCreateInformationInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__SpriteGlyphCreateInformationInterop ScriptSpriteGlyphCreateInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteGlyphCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteGlyphCreateInformation ScriptSpriteGlyphCreateInformation::FromInterop(const __SpriteGlyphCreateInformationInterop& value)
	{
		SpriteGlyphCreateInformation output;
		TResourceHandle<Font> tmpFont;
		ScriptRRefBase* scriptFont;
		scriptFont = ScriptRRefBase::ToNative(value.Font);
		if(scriptFont != nullptr)
			tmpFont = B3DStaticResourceCast<Font>(scriptFont->GetHandle());
		output.Font = tmpFont;
		output.Glyph = value.Glyph;
		output.Size = value.Size;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteGlyphCreateInformationInterop ScriptSpriteGlyphCreateInformation::ToInterop(const SpriteGlyphCreateInformation& value)
	{
		__SpriteGlyphCreateInformationInterop output;
		MonoObject* tmpFont;
		ScriptRRefBase* scriptFont;
		scriptFont = ScriptResourceManager::Instance().GetScriptRRef(value.Font);
		if(scriptFont != nullptr)
			tmpFont = scriptFont->GetManagedInstance();
		else
			tmpFont = nullptr;
		output.Font = tmpFont;
		output.Glyph = value.Glyph;
		output.Size = value.Size;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
