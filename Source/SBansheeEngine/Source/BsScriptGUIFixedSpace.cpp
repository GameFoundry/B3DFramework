//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIFixedSpace.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUISpace.h"
#include "BsScriptGUILayout.h"

namespace bs
{
	ScriptGUIFixedSpace::ScriptGUIFixedSpace(MonoObject* instance, GUIFixedSpace* fixedSpace)
		:TScriptGUIElementBase(instance, fixedSpace), mFixedSpace(fixedSpace), mIsDestroyed(false)
	{

	}

	void ScriptGUIFixedSpace::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIFixedSpace::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetSize", &ScriptGUIFixedSpace::internal_setSize);
	}

	void ScriptGUIFixedSpace::destroy()
	{
		if (!mIsDestroyed)
		{
			if (mParent != nullptr)
				mParent->removeChild(this);

			GUIFixedSpace::destroy(mFixedSpace);

			mIsDestroyed = true;
		}
	}

	void ScriptGUIFixedSpace::internal_createInstance(MonoObject* instance, UINT32 size)
	{
		GUIFixedSpace* space = GUIFixedSpace::create(size);

		new (bs_alloc<ScriptGUIFixedSpace>()) ScriptGUIFixedSpace(instance, space);
	}

	void ScriptGUIFixedSpace::internal_setSize(ScriptGUIFixedSpace* nativeInstance, UINT32 size)
	{
		nativeInstance->mFixedSpace->setSize(size);
	}
}