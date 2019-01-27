//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIFixedSpace.h"
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
	ScriptGUIFixedSpace::ScriptGUIFixedSpace(MonoObject* instance, GUIFixedSpace* fixedSpace)
		:TScriptGUIElementBase(instance, fixedSpace), mFixedSpace(fixedSpace), mIsDestroyed(false)
	{

	}

	void ScriptGUIFixedSpace::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIFixedSpace::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetSize", (void*)&ScriptGUIFixedSpace::internal_setSize);
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