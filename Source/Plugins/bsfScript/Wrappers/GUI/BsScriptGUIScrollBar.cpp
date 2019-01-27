//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIScrollBar.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUIScrollBar.h"
#include "GUI/BsGUIScrollBarHorz.h"
#include "GUI/BsGUIScrollBarVert.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIScrollBar::ScriptGUIScrollBar(MonoObject* instance)
		:TScriptGUIElement(instance, nullptr)
	{

	}

	void ScriptGUIScrollBar::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIScrollBar::internal_SetTint);
		metaData.scriptClass->addInternalCall("Internal_GetPosition", (void*)&ScriptGUIScrollBar::internal_GetPosition);
		metaData.scriptClass->addInternalCall("Internal_SetPosition", (void*)&ScriptGUIScrollBar::internal_SetPosition);
		metaData.scriptClass->addInternalCall("Internal_GetHandleSize", (void*)&ScriptGUIScrollBar::internal_GetHandleSize);
		metaData.scriptClass->addInternalCall("Internal_SetHandleSize", (void*)&ScriptGUIScrollBar::internal_SetHandleSize);
	}

	float ScriptGUIScrollBar::internal_GetPosition(ScriptGUIElementBaseTBase* nativeInstance)
	{
		GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->getGUIElement();
		return scrollBar->getScrollPos();
	}

	void ScriptGUIScrollBar::internal_SetPosition(ScriptGUIElementBaseTBase* nativeInstance, float percent)
	{
		GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->getGUIElement();
		scrollBar->_setScrollPos(percent);
	}

	float ScriptGUIScrollBar::internal_GetHandleSize(ScriptGUIElementBaseTBase* nativeInstance)
	{
		GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->getGUIElement();
		return scrollBar->getHandleSize();
	}

	void ScriptGUIScrollBar::internal_SetHandleSize(ScriptGUIElementBaseTBase* nativeInstance, float percent)
	{
		GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->getGUIElement();
		scrollBar->setHandleSize(percent);
	}

	void ScriptGUIScrollBar::internal_SetTint(ScriptGUIElementBaseTBase* nativeInstance, Color* color)
	{
		GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->getGUIElement();
		scrollBar->setTint(*color);
	}

	ScriptGUIScrollBarH::OnScrolledThunkDef ScriptGUIScrollBarH::onScrolledThunk;

	ScriptGUIScrollBarH::ScriptGUIScrollBarH(MonoObject* instance, GUIScrollBarHorz* scrollBar)
		:TScriptGUIElement(instance, scrollBar)
	{

	}

	void ScriptGUIScrollBarH::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIScrollBarH::internal_CreateInstance);

		onScrolledThunk = (OnScrolledThunkDef)metaData.scriptClass->getMethod("Internal_DoOnScroll", 1)->getThunk();
	}

	void ScriptGUIScrollBarH::internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIScrollBarHorz* guiScrollBar = GUIScrollBarHorz::create(options, MonoUtil::monoToString(style));
		ScriptGUIScrollBarH* scriptScrollBar = new (bs_alloc<ScriptGUIScrollBarH>()) ScriptGUIScrollBarH(instance, guiScrollBar);

		guiScrollBar->onScrollOrResize.connect(std::bind(&ScriptGUIScrollBarH::onScroll, scriptScrollBar, _1, _2));
	}

	void ScriptGUIScrollBarH::onScroll(float position, float size)
	{
		MonoUtil::invokeThunk(onScrolledThunk, getManagedInstance(), position);
	}

	ScriptGUIScrollBarV::OnScrolledThunkDef ScriptGUIScrollBarV::onScrolledThunk;

	ScriptGUIScrollBarV::ScriptGUIScrollBarV(MonoObject* instance, GUIScrollBarVert* scrollBar)
		:TScriptGUIElement(instance, scrollBar)
	{

	}

	void ScriptGUIScrollBarV::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIScrollBarV::internal_CreateInstance);

		onScrolledThunk = (OnScrolledThunkDef)metaData.scriptClass->getMethod("Internal_DoOnScroll", 1)->getThunk();
	}

	void ScriptGUIScrollBarV::internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIScrollBarVert* guiScrollBar = GUIScrollBarVert::create(options, MonoUtil::monoToString(style));
		ScriptGUIScrollBarV* scriptScrollBar = new (bs_alloc<ScriptGUIScrollBarV>()) ScriptGUIScrollBarV(instance, guiScrollBar);

		guiScrollBar->onScrollOrResize.connect(std::bind(&ScriptGUIScrollBarV::onScroll, scriptScrollBar, _1, _2));
	}

	void ScriptGUIScrollBarV::onScroll(float position, float size)
	{
		MonoUtil::invokeThunk(onScrolledThunk, getManagedInstance(), position);
	}

	ScriptGUIResizeableScrollBarH::OnScrollOrResizeThunkDef ScriptGUIResizeableScrollBarH::onScrollOrResizeThunk;

	ScriptGUIResizeableScrollBarH::ScriptGUIResizeableScrollBarH(MonoObject* instance, GUIScrollBarHorz* scrollBar)
		:TScriptGUIElement(instance, scrollBar)
	{

	}

	void ScriptGUIResizeableScrollBarH::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIResizeableScrollBarH::internal_CreateInstance);

		onScrollOrResizeThunk = (OnScrollOrResizeThunkDef)metaData.scriptClass->getMethod("Internal_DoOnScrollOrResize", 2)->getThunk();
	}

	void ScriptGUIResizeableScrollBarH::internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIScrollBarHorz* guiScrollBar = GUIScrollBarHorz::create(true, options, MonoUtil::monoToString(style));
		ScriptGUIResizeableScrollBarH* scriptScrollBar = new (bs_alloc<ScriptGUIResizeableScrollBarH>()) ScriptGUIResizeableScrollBarH(instance, guiScrollBar);

		guiScrollBar->onScrollOrResize.connect(std::bind(&ScriptGUIResizeableScrollBarH::onScroll, scriptScrollBar, _1, _2));
	}

	void ScriptGUIResizeableScrollBarH::onScroll(float position, float size)
	{
		MonoUtil::invokeThunk(onScrollOrResizeThunk, getManagedInstance(), position, size);
	}

	ScriptGUIResizeableScrollBarV::OnScrollOrResizeThunkDef ScriptGUIResizeableScrollBarV::onScrollOrResizeThunk;

	ScriptGUIResizeableScrollBarV::ScriptGUIResizeableScrollBarV(MonoObject* instance, GUIScrollBarVert* scrollBar)
		:TScriptGUIElement(instance, scrollBar)
	{

	}

	void ScriptGUIResizeableScrollBarV::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIResizeableScrollBarV::internal_CreateInstance);

		onScrollOrResizeThunk = (OnScrollOrResizeThunkDef)metaData.scriptClass->getMethod("Internal_DoOnScrollOrResize", 2)->getThunk();
	}

	void ScriptGUIResizeableScrollBarV::internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIScrollBarVert* guiScrollBar = GUIScrollBarVert::create(true, options, MonoUtil::monoToString(style));
		ScriptGUIResizeableScrollBarV* scriptScrollBar = new (bs_alloc<ScriptGUIResizeableScrollBarV>()) ScriptGUIResizeableScrollBarV(instance, guiScrollBar);

		guiScrollBar->onScrollOrResize.connect(std::bind(&ScriptGUIResizeableScrollBarV::onScroll, scriptScrollBar, _1, _2));
	}

	void ScriptGUIResizeableScrollBarV::onScroll(float position, float size)
	{
		MonoUtil::invokeThunk(onScrollOrResizeThunk, getManagedInstance(), position, size);
	}
}