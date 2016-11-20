//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptOSDropTarget.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsMonoArray.h"
#include "BsRTTIType.h"
#include "BsPlatform.h"
#include "BsEditorWidget.h"
#include "BsEditorWindowBase.h"
#include "BsEditorWidgetContainer.h"
#include "BsScriptEditorWindow.h"

using namespace std::placeholders;

namespace bs
{
	ScriptOSDropTarget::OnEnterThunkDef ScriptOSDropTarget::onEnterThunk;
	ScriptOSDropTarget::OnMoveDef ScriptOSDropTarget::onMoveThunk;
	ScriptOSDropTarget::OnLeaveDef ScriptOSDropTarget::onLeaveThunk;
	ScriptOSDropTarget::OnDropThunkDef ScriptOSDropTarget::onDropThunk;

	ScriptOSDropTarget::ScriptOSDropTarget(MonoObject* instance, ScriptEditorWindow* parent)
		:ScriptObject(instance), mParent(parent), mDropTarget(nullptr), mIsDestroyed(false)
	{
		EditorWidgetBase* parentWidget = getParentWidget();

		if (parentWidget != nullptr)
		{
			mWidgetParentChangedConn = parentWidget->onParentChanged.connect(std::bind(&ScriptOSDropTarget::widgetParentChanged, this, _1));
			mWidgetResizedConn = parentWidget->onResized.connect(std::bind(&ScriptOSDropTarget::widgetResized, this, _1, _2));
			mWidgetMovedConn = parentWidget->onMoved.connect(std::bind(&ScriptOSDropTarget::widgetMoved, this, _1, _2));

			EditorWindowBase* parentWindow = parentWidget->getParentWindow();

			if (parentWindow != nullptr)
				setDropTarget(parentWindow->getRenderWindow(), 0, 0, 0, 0);

			mParentArea = parentWidget->getBounds();
		}
	}

	ScriptOSDropTarget::~ScriptOSDropTarget()
	{
		if (!mIsDestroyed)
			destroy();
	}

	void ScriptOSDropTarget::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptOSDropTarget::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptOSDropTarget::internal_Destroy);
		metaData.scriptClass->addInternalCall("Internal_SetBounds", &ScriptOSDropTarget::internal_SetBounds);
		metaData.scriptClass->addInternalCall("Internal_GetFilePaths", &ScriptOSDropTarget::internal_GetFilePaths);

		onEnterThunk = (OnEnterThunkDef)metaData.scriptClass->getMethod("InternalDoOnEnter", 2)->getThunk();
		onMoveThunk = (OnMoveDef)metaData.scriptClass->getMethod("InternalDoOnDrag", 2)->getThunk();
		onLeaveThunk = (OnLeaveDef)metaData.scriptClass->getMethod("InternalDoOnLeave")->getThunk();
		onDropThunk = (OnDropThunkDef)metaData.scriptClass->getMethod("InternalDoOnDrop", 2)->getThunk();
	}

	void ScriptOSDropTarget::internal_CreateInstance(MonoObject* instance, ScriptEditorWindow* editorWindow)
	{
		new (bs_alloc<ScriptOSDropTarget>()) ScriptOSDropTarget(instance, editorWindow);
	}

	void ScriptOSDropTarget::internal_Destroy(ScriptOSDropTarget* nativeInstance)
	{
		if (nativeInstance->mIsDestroyed)
			return;

		nativeInstance->destroy();
	}

	void ScriptOSDropTarget::internal_SetBounds(ScriptOSDropTarget* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->mIsDestroyed)
			return;

		nativeInstance->setBounds(*bounds);
	}

	MonoArray* ScriptOSDropTarget::internal_GetFilePaths(ScriptOSDropTarget* nativeInstance)
	{
		OSDropTarget* dropTarget = nativeInstance->mDropTarget;

		if (nativeInstance->mIsDestroyed || dropTarget == nullptr || dropTarget->getDropType() != OSDropType::FileList)
			return ScriptArray::create<String>(0).getInternal();

		Vector<WString> fileList = dropTarget->getFileList();
		ScriptArray output = ScriptArray::create<WString>((UINT32)fileList.size());

		UINT32 idx = 0;
		for (auto& path : fileList)
		{
			output.set(idx, path);
			idx++;
		}

		return output.getInternal();
	}

	void ScriptOSDropTarget::destroy()
	{
		mIsDestroyed = true;

		mWidgetParentChangedConn.disconnect();
		mWidgetResizedConn.disconnect();

		setDropTarget(nullptr, 0, 0, 0, 0);
	}

	EditorWidgetBase* ScriptOSDropTarget::getParentWidget() const
	{
		EditorWidgetBase* parentWidget = nullptr;

		if (mParent != nullptr && !mParent->isDestroyed())
			parentWidget = mParent->getEditorWidget();

		return parentWidget;
	}

	Rect2I ScriptOSDropTarget::getDropTargetArea() const
	{
		Rect2I dropTargetArea = mArea;
		dropTargetArea.x += mParentArea.x;
		dropTargetArea.y += mParentArea.y;

		dropTargetArea.clip(mParentArea);

		return dropTargetArea;
	}

	void ScriptOSDropTarget::setDropTarget(const SPtr<RenderWindow>& parentWindow, INT32 x, INT32 y, UINT32 width, UINT32 height)
	{
		if (mDropTarget != nullptr)
		{
			Platform::destroyDropTarget(*mDropTarget);

			mDropTargetEnterConn.disconnect();
			mDropTargetLeaveConn.disconnect();
			mDropTargetMoveConn.disconnect();
			mDropTargetDroppedConn.disconnect();
		}

		if (parentWindow != nullptr)
		{
			mDropTarget = &Platform::createDropTarget(parentWindow.get(), x, y, width, height);

			mDropTargetEnterConn = mDropTarget->onEnter.connect(std::bind(&ScriptOSDropTarget::dropTargetDragEnter, this, _1, _2));
			mDropTargetMoveConn = mDropTarget->onDragOver.connect(std::bind(&ScriptOSDropTarget::dropTargetDragMove, this, _1, _2));
			mDropTargetLeaveConn = mDropTarget->onLeave.connect(std::bind(&ScriptOSDropTarget::dropTargetDragLeave, this));
			mDropTargetDroppedConn = mDropTarget->onDrop.connect(std::bind(&ScriptOSDropTarget::dropTargetDragDropped, this, _1, _2));
		}
		else
			mDropTarget = nullptr;
	}

	void ScriptOSDropTarget::setBounds(const Rect2I& bounds)
	{
		mArea = bounds;
		Rect2I dropTargetArea = getDropTargetArea();

		if (mDropTarget != nullptr)
			mDropTarget->setArea(dropTargetArea.x, dropTargetArea.y, dropTargetArea.width, dropTargetArea.height);
	}

	void ScriptOSDropTarget::dropTargetDragEnter(ScriptOSDropTarget* thisPtr, INT32 x, INT32 y)
	{
		if (thisPtr->mIsDestroyed)
			return;

		MonoUtil::invokeThunk(onEnterThunk, thisPtr->getManagedInstance(), 
			x - thisPtr->mParentArea.x, y - thisPtr->mParentArea.y);
	}

	void ScriptOSDropTarget::dropTargetDragMove(ScriptOSDropTarget* thisPtr, INT32 x, INT32 y)
	{
		if (thisPtr->mIsDestroyed)
			return;

		MonoUtil::invokeThunk(onMoveThunk, thisPtr->getManagedInstance(), 
			x - thisPtr->mParentArea.x, y - thisPtr->mParentArea.y);
	}

	void ScriptOSDropTarget::dropTargetDragLeave(ScriptOSDropTarget* thisPtr)
	{
		if (thisPtr->mIsDestroyed)
			return;

		MonoUtil::invokeThunk(onLeaveThunk, thisPtr->getManagedInstance());
	}

	void ScriptOSDropTarget::dropTargetDragDropped(ScriptOSDropTarget* thisPtr, INT32 x, INT32 y)
	{
		if (thisPtr->mIsDestroyed)
			return;

		MonoUtil::invokeThunk(onDropThunk, thisPtr->getManagedInstance(), 
			x - thisPtr->mParentArea.x, y - thisPtr->mParentArea.y);
	}

	void ScriptOSDropTarget::widgetParentChanged(EditorWidgetContainer* parent)
	{
		SPtr<RenderWindow> parentRenderWindow;
		if (parent != nullptr)
		{
			EditorWindowBase* parentWindow = parent->getParentWindow();

			if (parentWindow != nullptr)
				parentRenderWindow = parentWindow->getRenderWindow();
		}

		if (parentRenderWindow == nullptr)
			mParentArea = Rect2I();
		
		Rect2I dropTargetArea = getDropTargetArea();
		setDropTarget(parentRenderWindow, dropTargetArea.x, dropTargetArea.y, dropTargetArea.width, dropTargetArea.height);
	}

	void ScriptOSDropTarget::widgetResized(UINT32 width, UINT32 height)
	{
		mParentArea.width = width;
		mParentArea.height = height;

		setBounds(mArea);
	}

	void ScriptOSDropTarget::widgetMoved(INT32 x, INT32 y)
	{
		mParentArea.x = x;
		mParentArea.y = y;

		setBounds(mArea);
	}
}