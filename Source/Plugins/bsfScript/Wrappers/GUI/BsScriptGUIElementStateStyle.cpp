//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIElementStateStyle.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsScriptResourceManager.h"
#include "GUI/BsGUIElementStyle.h"
#include "Wrappers/BsScriptColor.h"

#include "BsScriptSpriteTexture.generated.h"

namespace bs
{
	ScriptGUIElementStateStyle::ScriptGUIElementStateStyle(MonoObject* instance)
		:ScriptObject(instance)
	{

	}

	void ScriptGUIElementStateStyle::initRuntimeData()
	{

	}

	ScriptGUIElementStateStyleStruct ScriptGUIElementStateStyle::toManaged(const GUIElementStyle::GUIElementStateStyle& state)
	{
		ScriptResourceBase* scriptTexture = nullptr;

		if (state.texture != nullptr)
			scriptTexture = ScriptResourceManager::instance().getScriptResource(state.texture, true);

		ScriptGUIElementStateStyleStruct output;
		output.texture = scriptTexture != nullptr ? scriptTexture->getManagedInstance() : nullptr;
		output.textColor = state.textColor;

		return output;
	}

	GUIElementStyle::GUIElementStateStyle ScriptGUIElementStateStyle::toNative(const ScriptGUIElementStateStyleStruct& instance)
	{
		GUIElementStyle::GUIElementStateStyle output;

		if (instance.texture != nullptr)
		{
			ScriptSpriteTexture* scriptTexture = ScriptSpriteTexture::toNative(instance.texture);
			output.texture = scriptTexture->getHandle();
		}

		output.textColor = instance.textColor;
		return output;
	}
}