//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIUtility.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUtility.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptTSize2.generated.h"
#include "../../../Foundation/Core/Text/B3DFont.h"

namespace b3d
{
	ScriptGUIUtility::ScriptGUIUtility()
		:TScriptTypeDefinition()
	{
	}

	void ScriptGUIUtility::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateTextBounds", (void*)&ScriptGUIUtility::InternalCalculateTextBounds);

	}

	void ScriptGUIUtility::InternalCalculateTextBounds(MonoString* text, MonoObject* font, float fontSize, TSize2<int32_t>* __output)
	{
		String tmptext;
		tmptext = MonoUtil::MonoToString(text);
		TResourceHandle<Font> tmpfont;
		ScriptRRefBase* scriptObjectWrapperfont;
		scriptObjectWrapperfont = ScriptRRefBase::GetScriptObjectWrapper(font);
		if(scriptObjectWrapperfont != nullptr)
			tmpfont = B3DStaticResourceCast<Font>(scriptObjectWrapperfont->GetNativeObject());
		TSize2<int32_t> tmp__output;
		tmp__output = GUIUtility::CalculateTextBounds(tmptext, tmpfont, fontSize);

		*__output = tmp__output;
	}
}
