//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptEditorBuiltin.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptGUIContentImages.h"
#include "BsScriptResourceManager.h"
#include "BsScriptFont.h"

namespace BansheeEngine
{
	ScriptEditorBuiltin::ScriptEditorBuiltin(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptEditorBuiltin::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetLibraryItemIcon", &ScriptEditorBuiltin::internal_getLibraryItemIcon);
		metaData.scriptClass->addInternalCall("Internal_GetEmptyShaderCode", &ScriptEditorBuiltin::internal_GetEmptyShaderCode);
		metaData.scriptClass->addInternalCall("Internal_GetEmptyCSScriptCode", &ScriptEditorBuiltin::internal_GetEmptyCSScriptCode);
		metaData.scriptClass->addInternalCall("Internal_GetToolbarIcon", &ScriptEditorBuiltin::internal_GetToolbarIcon);
		metaData.scriptClass->addInternalCall("Internal_GetLibraryWindowIcon", &ScriptEditorBuiltin::internal_GetLibraryWindowIcon);
		metaData.scriptClass->addInternalCall("Internal_GetInspectorWindowIcon", &ScriptEditorBuiltin::internal_GetInspectorWindowIcon);
		metaData.scriptClass->addInternalCall("Internal_GetSceneWindowIcon", &ScriptEditorBuiltin::internal_GetSceneWindowIcon);
		metaData.scriptClass->addInternalCall("Internal_GetAnimationWindowIcon", &ScriptEditorBuiltin::internal_GetAnimationWindowIcon);
		metaData.scriptClass->addInternalCall("Internal_GetLogIcon", &ScriptEditorBuiltin::internal_GetLogIcon);
		metaData.scriptClass->addInternalCall("Internal_GetEditorIcon", &ScriptEditorBuiltin::internal_GetEditorIcon);
		metaData.scriptClass->addInternalCall("Internal_GetDefaultFont", &ScriptEditorBuiltin::internal_GetDefaultFont);
	}

	MonoObject* ScriptEditorBuiltin::internal_getLibraryItemIcon(ProjectIcon icon, int size)
	{
		HSpriteTexture tex = BuiltinEditorResources::instance().getLibraryIcon(icon, size);

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoString* ScriptEditorBuiltin::internal_GetEmptyShaderCode()
	{
		WString code = BuiltinEditorResources::instance().getEmptyShaderCode();

		return MonoUtil::wstringToMono(code);
	}

	MonoString* ScriptEditorBuiltin::internal_GetEmptyCSScriptCode()
	{
		WString code = BuiltinEditorResources::instance().getEmptyCSScriptCode();

		return MonoUtil::wstringToMono(code);
	}

	MonoObject* ScriptEditorBuiltin::internal_GetToolbarIcon(ToolbarIcon icon)
	{
		HSpriteTexture tex = BuiltinEditorResources::instance().getToolbarIcon(icon);

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoObject* ScriptEditorBuiltin::internal_GetLibraryWindowIcon(LibraryWindowIcon icon)
	{
		HSpriteTexture tex = BuiltinEditorResources::instance().getLibraryWindowIcon(icon);

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoObject* ScriptEditorBuiltin::internal_GetInspectorWindowIcon(InspectorWindowIcon icon)
	{
		HSpriteTexture tex = BuiltinEditorResources::instance().getInspectorWindowIcon(icon);

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoObject* ScriptEditorBuiltin::internal_GetSceneWindowIcon(SceneWindowIcon icon)
	{
		GUIContentImages images = BuiltinEditorResources::instance().getSceneWindowIcon(icon);

		return ScriptGUIContentImages::getManaged(images);
	}

	MonoObject* ScriptEditorBuiltin::internal_GetAnimationWindowIcon(AnimationWindowIcon icon)
	{
		GUIContentImages images = BuiltinEditorResources::instance().getAnimationWindowIcon(icon);

		return ScriptGUIContentImages::getManaged(images);
	}

	MonoObject* ScriptEditorBuiltin::internal_GetLogIcon(LogMessageIcon icon, int size, bool dark)
	{
		HSpriteTexture tex = BuiltinEditorResources::instance().getLogMessageIcon(icon, size, dark);

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoObject* ScriptEditorBuiltin::internal_GetEditorIcon(EditorIcon icon)
	{
		HSpriteTexture tex = BuiltinEditorResources::instance().getIcon(icon);

		return ScriptSpriteTexture::toManaged(tex);
	}
	
	MonoObject* ScriptEditorBuiltin::internal_GetDefaultFont()
	{
		HFont font = BuiltinEditorResources::instance().getDefaultFont();

		ScriptFont* scriptFont;
		ScriptResourceManager::instance().getScriptResource(font, &scriptFont, true);

		return scriptFont->getManagedInstance();
	}
}