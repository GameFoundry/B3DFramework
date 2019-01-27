//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIFlexibleSpace.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUISpace.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

namespace bs
{
	ScriptGUIFlexibleSpace::ScriptGUIFlexibleSpace(MonoObject* instance, GUIFlexibleSpace* flexibleSpace)
		:TScriptGUIElementBase(instance, flexibleSpace), mFlexibleSpace(flexibleSpace), mIsDestroyed(false)
	{

	}

	void ScriptGUIFlexibleSpace::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIFlexibleSpace::internal_createInstance);
	}

	void ScriptGUIFlexibleSpace::destroy()
	{
		if(!mIsDestroyed)
		{
			if (mParent != nullptr)
				mParent->removeChild(this);

			GUIFlexibleSpace::destroy(mFlexibleSpace);

			mIsDestroyed = true;
		}
	}

	void ScriptGUIFlexibleSpace::internal_createInstance(MonoObject* instance)
	{
		GUIFlexibleSpace* space = GUIFlexibleSpace::create();

		new (bs_alloc<ScriptGUIFlexibleSpace>()) ScriptGUIFlexibleSpace(instance, space);
	}
}