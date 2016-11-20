//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUISceneTreeView.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsGUISceneTreeView.h"
#include "BsGUIOptions.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptSceneObject.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUISceneTreeView::OnModifiedThunkDef ScriptGUISceneTreeView::onModifiedThunk;
	ScriptGUISceneTreeView::OnResourceDroppedThunkDef ScriptGUISceneTreeView::onResourceDroppedThunk;

	ScriptGUISceneTreeView::ScriptGUISceneTreeView(MonoObject* instance, GUISceneTreeView* treeView)
		:TScriptGUIElement(instance, treeView)
	{
		mOnModifiedConn = treeView->onModified.connect(std::bind(&ScriptGUISceneTreeView::sceneModified, this));
		mOnResourceDroppedConn = treeView->onResourceDropped.connect(
			std::bind(&ScriptGUISceneTreeView::resourceDropped, this, _1, _2));
	}

	ScriptGUISceneTreeView::~ScriptGUISceneTreeView()
	{
		mOnModifiedConn.disconnect();
		mOnResourceDroppedConn.disconnect();
	}

	void ScriptGUISceneTreeView::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUISceneTreeView::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_Update", &ScriptGUISceneTreeView::internal_update);
		metaData.scriptClass->addInternalCall("Internal_CutSelection", &ScriptGUISceneTreeView::internal_cutSelection);
		metaData.scriptClass->addInternalCall("Internal_CopySelection", &ScriptGUISceneTreeView::internal_copySelection);
		metaData.scriptClass->addInternalCall("Internal_PasteToSelection", &ScriptGUISceneTreeView::internal_pasteToSelection);
		metaData.scriptClass->addInternalCall("Internal_DuplicateSelection", &ScriptGUISceneTreeView::internal_duplicateSelection);
		metaData.scriptClass->addInternalCall("Internal_DeleteSelection", &ScriptGUISceneTreeView::internal_deleteSelection);
		metaData.scriptClass->addInternalCall("Internal_RenameSelection", &ScriptGUISceneTreeView::internal_renameSelection);

		onModifiedThunk = (OnModifiedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnModified", 0)->getThunk();
		onResourceDroppedThunk = (OnResourceDroppedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnResourceDropped", 2)->getThunk();
	}

	void ScriptGUISceneTreeView::sceneModified()
	{
		MonoUtil::invokeThunk(onModifiedThunk, getManagedInstance());
	}

	void ScriptGUISceneTreeView::resourceDropped(const HSceneObject& parent, const Vector<Path>& resourcePaths)
	{
		MonoObject* sceneMonoObject = nullptr;

		if (parent != nullptr)
		{
			ScriptSceneObject* scriptSceneObject = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(parent);
			sceneMonoObject = scriptSceneObject->getManagedInstance();
		}

		UINT32 numPaths = (UINT32)resourcePaths.size();
		ScriptArray array = ScriptArray::create<WString>(numPaths);
		for (UINT32 i = 0; i < numPaths; i++)
			array.set(i, resourcePaths[i].toWString());

		MonoUtil::invokeThunk(onResourceDroppedThunk, getManagedInstance(), sceneMonoObject, array.getInternal());
	}

	void ScriptGUISceneTreeView::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUISceneTreeView* treeView = GUISceneTreeView::create(options);
		new (bs_alloc<ScriptGUISceneTreeView>()) ScriptGUISceneTreeView(instance, treeView);
	}

	void ScriptGUISceneTreeView::internal_update(ScriptGUISceneTreeView* thisPtr)
	{
		if (thisPtr->mIsDestroyed)
			return;

		GUISceneTreeView* treeView = static_cast<GUISceneTreeView*>(thisPtr->getGUIElement());
		treeView->_update();
	}

	void ScriptGUISceneTreeView::internal_cutSelection(ScriptGUISceneTreeView* thisPtr)
	{
		GUISceneTreeView* treeView = static_cast<GUISceneTreeView*>(thisPtr->getGUIElement());
		treeView->cutSelection();
	}

	void ScriptGUISceneTreeView::internal_copySelection(ScriptGUISceneTreeView* thisPtr)
	{
		GUISceneTreeView* treeView = static_cast<GUISceneTreeView*>(thisPtr->getGUIElement());
		treeView->copySelection();
	}

	void ScriptGUISceneTreeView::internal_pasteToSelection(ScriptGUISceneTreeView* thisPtr)
	{
		GUISceneTreeView* treeView = static_cast<GUISceneTreeView*>(thisPtr->getGUIElement());
		treeView->paste();
	}

	void ScriptGUISceneTreeView::internal_duplicateSelection(ScriptGUISceneTreeView* thisPtr)
	{
		GUISceneTreeView* treeView = static_cast<GUISceneTreeView*>(thisPtr->getGUIElement());
		treeView->duplicateSelection();
	}

	void ScriptGUISceneTreeView::internal_deleteSelection(ScriptGUISceneTreeView* thisPtr)
	{
		GUISceneTreeView* treeView = static_cast<GUISceneTreeView*>(thisPtr->getGUIElement());
		treeView->deleteSelection();
	}

	void ScriptGUISceneTreeView::internal_renameSelection(ScriptGUISceneTreeView* thisPtr)
	{
		GUISceneTreeView* treeView = static_cast<GUISceneTreeView*>(thisPtr->getGUIElement());
		treeView->renameSelected();
	}
}