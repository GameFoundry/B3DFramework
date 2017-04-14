//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIElementStateStyle.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptResourceManager.h"
#include "BsGUIElementStyle.h"
#include "BsScriptColor.h"

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