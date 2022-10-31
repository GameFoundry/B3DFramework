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

using namespace bs;
ScriptGUIFlexibleSpace::ScriptGUIFlexibleSpace(MonoObject* instance, GUIFlexibleSpace* flexibleSpace)
	: TScriptGUIElementBase(instance, flexibleSpace), mFlexibleSpace(flexibleSpace), mIsDestroyed(false)
{
}

void ScriptGUIFlexibleSpace::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIFlexibleSpace::InternalCreateInstance);
}

void ScriptGUIFlexibleSpace::Destroy()
{
	if(!mIsDestroyed)
	{
		if(mParent != nullptr)
			mParent->RemoveChild(this);

		GUIFlexibleSpace::Destroy(mFlexibleSpace);

		mIsDestroyed = true;
	}
}

void ScriptGUIFlexibleSpace::InternalCreateInstance(MonoObject* instance)
{
	GUIFlexibleSpace* space = GUIFlexibleSpace::Create();

	new(B3DAllocate<ScriptGUIFlexibleSpace>()) ScriptGUIFlexibleSpace(instance, space);
}
