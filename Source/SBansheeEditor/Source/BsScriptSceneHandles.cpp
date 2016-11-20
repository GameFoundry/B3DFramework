//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSceneHandles.h"
#include "BsScriptCamera.h"
#include "BsScriptEditorWindow.h"
#include "BsCursor.h"
#include "BsRenderWindow.h"
#include "BsEditorWidget.h"
#include "BsEditorWindow.h"
#include "BsHandleManager.h"
#include "BsEditorApplication.h"

namespace bs
{
	ScriptSceneHandles::ScriptSceneHandles(MonoObject* object, EditorWidgetBase* parentWidget, const SPtr<Camera>& camera)
		:ScriptObject(object), mParentWidget(parentWidget), mCamera(camera)
	{
		HandleManager::instance().setSettings(gEditorApplication().getEditorSettings());
	}

	ScriptSceneHandles::~ScriptSceneHandles()
	{

	}

	void ScriptSceneHandles::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptSceneHandles::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_Draw", &ScriptSceneHandles::internal_Draw);
		metaData.scriptClass->addInternalCall("Internal_BeginInput", &ScriptSceneHandles::internal_BeginInput);
		metaData.scriptClass->addInternalCall("Internal_EndInput", &ScriptSceneHandles::internal_EndInput);
		metaData.scriptClass->addInternalCall("Internal_UpdateInput", &ScriptSceneHandles::internal_UpdateInput);
		metaData.scriptClass->addInternalCall("Internal_TrySelect", &ScriptSceneHandles::internal_TrySelect);
		metaData.scriptClass->addInternalCall("Internal_IsActive", &ScriptSceneHandles::internal_IsActive);
		metaData.scriptClass->addInternalCall("Internal_ClearSelection", &ScriptSceneHandles::internal_ClearSelection);
	}

	Vector2I ScriptSceneHandles::wrapCursorToWindow() const
	{
		if (mParentWidget == nullptr)
			return Vector2I();

		SPtr<RenderWindow> parentWindow = mParentWidget->getParentWindow()->getRenderWindow();

		Vector2I windowPos = parentWindow->screenToWindowPos(Cursor::instance().getScreenPosition());
		const RenderWindowProperties& rwProps = parentWindow->getProperties();

		INT32 maxWidth = std::max(0, (INT32)rwProps.getWidth() - 1);
		INT32 maxHeight = std::max(0, (INT32)rwProps.getHeight() - 1);

		Vector2I offset;
		if (windowPos.x <= 0)
			offset.x = maxWidth;
		else if (windowPos.x >= maxWidth)
			offset.x = -maxWidth;

		if (windowPos.y <= 0)
			offset.y = maxHeight;
		else if (windowPos.y >= maxHeight)
			offset.y = -maxHeight;

		windowPos += offset;

		Vector2I wrappedScreenPos = parentWindow->windowToScreenPos(windowPos);
		Cursor::instance().setScreenPosition(wrappedScreenPos);

		return offset;
	}

	void ScriptSceneHandles::internal_Create(MonoObject* managedInstance, ScriptEditorWindow* parentWindow, ScriptCamera* camera)
	{
		EditorWidgetBase* widget = nullptr;

		if (parentWindow != nullptr && !parentWindow->isDestroyed())
			widget = parentWindow->getEditorWidget();

		new (bs_alloc<ScriptSceneHandles>()) ScriptSceneHandles(managedInstance, widget, camera->getInternal());
	}

	void ScriptSceneHandles::internal_Draw(ScriptSceneHandles* thisPtr)
	{
		HandleManager::instance().draw(thisPtr->mCamera);
	}

	void ScriptSceneHandles::internal_BeginInput()
	{
		HandleManager::instance().beginInput();
	}

	void ScriptSceneHandles::internal_EndInput()
	{
		HandleManager::instance().endInput();
	}

	void ScriptSceneHandles::internal_UpdateInput(ScriptSceneHandles* thisPtr, Vector2I* inputPos, Vector2I* inputDelta)
	{
		// If mouse wrapped around last frame then we need to compensate for the jump amount
		Vector2I realDelta = *inputDelta - thisPtr->mMouseDeltaCompensate;
		thisPtr->mMouseDeltaCompensate = Vector2I::ZERO;

		if (HandleManager::instance().isHandleActive(thisPtr->mCamera))
			thisPtr->mMouseDeltaCompensate = thisPtr->wrapCursorToWindow();

		HandleManager::instance().updateInput(thisPtr->mCamera, *inputPos, realDelta);
	}

	void ScriptSceneHandles::internal_TrySelect(ScriptSceneHandles* thisPtr, Vector2I* inputPos)
	{
		HandleManager::instance().trySelect(thisPtr->mCamera, *inputPos);
	}

	bool ScriptSceneHandles::internal_IsActive(ScriptSceneHandles* thisPtr)
	{
		return HandleManager::instance().isHandleActive(thisPtr->mCamera);
	}

	void ScriptSceneHandles::internal_ClearSelection(ScriptSceneHandles* thisPtr)
	{
		HandleManager::instance().clearSelection(thisPtr->mCamera);
	}
}