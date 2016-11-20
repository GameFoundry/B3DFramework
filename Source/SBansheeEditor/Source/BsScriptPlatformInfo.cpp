//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPlatformInfo.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsScriptTexture2D.h"
#include "BsScriptResourceManager.h"
#include "BsScriptPrefab.h"
#include "BsScriptResourceRef.h"

namespace bs
{
	ScriptPlatformInfoBase::ScriptPlatformInfoBase(MonoObject* instance)
		:ScriptObjectBase(instance)
	{ }

	ScriptPlatformInfo::ScriptPlatformInfo(MonoObject* instance)
		: ScriptObject(instance)
	{

	}

	void ScriptPlatformInfo::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetType", &ScriptPlatformInfo::internal_GetType);
		metaData.scriptClass->addInternalCall("Internal_GetDefines", &ScriptPlatformInfo::internal_GetDefines);
		metaData.scriptClass->addInternalCall("Internal_SetDefines", &ScriptPlatformInfo::internal_SetDefines);
		metaData.scriptClass->addInternalCall("Internal_GetMainScene", &ScriptPlatformInfo::internal_GetMainScene);
		metaData.scriptClass->addInternalCall("Internal_SetMainScene", &ScriptPlatformInfo::internal_SetMainScene);
		metaData.scriptClass->addInternalCall("Internal_GetFullscreen", &ScriptPlatformInfo::internal_GetFullscreen);
		metaData.scriptClass->addInternalCall("Internal_SetFullscreen", &ScriptPlatformInfo::internal_SetFullscreen);
		metaData.scriptClass->addInternalCall("Internal_GetResolution", &ScriptPlatformInfo::internal_GetResolution);
		metaData.scriptClass->addInternalCall("Internal_SetResolution", &ScriptPlatformInfo::internal_SetResolution);
		metaData.scriptClass->addInternalCall("Internal_GetDebug", &ScriptPlatformInfo::internal_GetDebug);
		metaData.scriptClass->addInternalCall("Internal_SetDebug", &ScriptPlatformInfo::internal_SetDebug);
	}

	MonoObject* ScriptPlatformInfo::create(const SPtr<PlatformInfo>& platformInfo)
	{
		switch (platformInfo->type)
		{
		case PlatformType::Windows:
			return ScriptWinPlatformInfo::create(std::static_pointer_cast<WinPlatformInfo>(platformInfo));
		default:
			break;
		}

		return nullptr;
	}

	PlatformType ScriptPlatformInfo::internal_GetType(ScriptPlatformInfoBase* thisPtr)
	{
		return thisPtr->getPlatformInfo()->type;
	}

	MonoString* ScriptPlatformInfo::internal_GetDefines(ScriptPlatformInfoBase* thisPtr)
	{
		return MonoUtil::wstringToMono(thisPtr->getPlatformInfo()->defines);
	}

	void ScriptPlatformInfo::internal_SetDefines(ScriptPlatformInfoBase* thisPtr, MonoString* value)
	{
		thisPtr->getPlatformInfo()->defines = MonoUtil::monoToWString(value);
	}

	MonoObject* ScriptPlatformInfo::internal_GetMainScene(ScriptPlatformInfoBase* thisPtr)
	{
		WeakResourceHandle<Prefab> prefab = thisPtr->getPlatformInfo()->mainScene;
		
		if (prefab != nullptr)
			return ScriptResourceRef::create(prefab);

		return nullptr;
	}

	void ScriptPlatformInfo::internal_SetMainScene(ScriptPlatformInfoBase* thisPtr, ScriptResourceRef* prefabRef)
	{
		WeakResourceHandle<Prefab> prefab;

		if (prefabRef != nullptr)
			prefab = static_resource_cast<Prefab>(prefabRef->getHandle());

		thisPtr->getPlatformInfo()->mainScene = prefab;
	}

	bool ScriptPlatformInfo::internal_GetFullscreen(ScriptPlatformInfoBase* thisPtr)
	{
		return thisPtr->getPlatformInfo()->fullscreen;
	}

	void ScriptPlatformInfo::internal_SetFullscreen(ScriptPlatformInfoBase* thisPtr, bool fullscreen)
	{
		thisPtr->getPlatformInfo()->fullscreen = fullscreen;
	}

	void ScriptPlatformInfo::internal_GetResolution(ScriptPlatformInfoBase* thisPtr, UINT32* width, UINT32* height)
	{
		*width = thisPtr->getPlatformInfo()->windowedWidth;
		*height = thisPtr->getPlatformInfo()->windowedHeight;
	}

	void ScriptPlatformInfo::internal_SetResolution(ScriptPlatformInfoBase* thisPtr, UINT32 width, UINT32 height)
	{
		thisPtr->getPlatformInfo()->windowedWidth = width;
		thisPtr->getPlatformInfo()->windowedHeight = height;
	}

	bool ScriptPlatformInfo::internal_GetDebug(ScriptPlatformInfoBase* thisPtr)
	{
		return thisPtr->getPlatformInfo()->debug;
	}

	void ScriptPlatformInfo::internal_SetDebug(ScriptPlatformInfoBase* thisPtr, bool debug)
	{
		thisPtr->getPlatformInfo()->debug = debug;
	}

	ScriptWinPlatformInfo::ScriptWinPlatformInfo(MonoObject* instance)
		:ScriptObject(instance)
	{
		mPlatformInfo = bs_shared_ptr_new<WinPlatformInfo>();
	}

	void ScriptWinPlatformInfo::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetIcon", &ScriptWinPlatformInfo::internal_GetIcon);
		metaData.scriptClass->addInternalCall("Internal_SetIcon", &ScriptWinPlatformInfo::internal_SetIcon);
		metaData.scriptClass->addInternalCall("Internal_GetTitleText", &ScriptWinPlatformInfo::internal_GetTitleText);
		metaData.scriptClass->addInternalCall("Internal_SetTitleText", &ScriptWinPlatformInfo::internal_SetTitleText);
	}

	SPtr<WinPlatformInfo> ScriptWinPlatformInfo::getWinPlatformInfo() const
	{
		return std::static_pointer_cast<WinPlatformInfo>(mPlatformInfo);
	}

	MonoObject* ScriptWinPlatformInfo::create(const SPtr<WinPlatformInfo>& platformInfo)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance();
		ScriptWinPlatformInfo* scriptObj = new (bs_alloc<ScriptWinPlatformInfo>()) ScriptWinPlatformInfo(managedInstance);
		scriptObj->mPlatformInfo = platformInfo;

		return managedInstance;
	}

	MonoObject* ScriptWinPlatformInfo::internal_GetIcon(ScriptWinPlatformInfo* thisPtr)
	{
		WeakResourceHandle<Texture> icon = thisPtr->getWinPlatformInfo()->icon;

		if (icon != nullptr)
			return ScriptResourceRef::create(icon, TEX_TYPE_2D);

		return nullptr;
	}

	void ScriptWinPlatformInfo::internal_SetIcon(ScriptWinPlatformInfo* thisPtr, ScriptResourceRef* textureRef)
	{
		WeakResourceHandle<Texture> icon;

		if (textureRef != nullptr)
			icon = static_resource_cast<Texture>(textureRef->getHandle());

		thisPtr->getWinPlatformInfo()->icon = icon;
	}

	MonoString* ScriptWinPlatformInfo::internal_GetTitleText(ScriptWinPlatformInfo* thisPtr)
	{
		WString titleText = thisPtr->getWinPlatformInfo()->titlebarText;

		return MonoUtil::wstringToMono(titleText);
	}

	void ScriptWinPlatformInfo::internal_SetTitleText(ScriptWinPlatformInfo* thisPtr, MonoString* text)
	{
		WString titleText = MonoUtil::monoToWString(text);

		thisPtr->getWinPlatformInfo()->titlebarText = titleText;
	}
}