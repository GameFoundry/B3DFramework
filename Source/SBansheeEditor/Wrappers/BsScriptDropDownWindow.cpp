//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptDropDownWindow.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoAssembly.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "Wrappers/BsScriptEditorWindow.h"
#include "EditorWindow/BsEditorWidget.h"
#include "EditorWindow/BsEditorWindow.h"
#include "EditorWindow/BsEditorWidgetContainer.h"
#include "GUI/BsCGUIWidget.h"
#include "EditorWindow/BsDropDownWindowManager.h"
#include "BsScriptObjectManager.h"

using namespace std::placeholders;

namespace bs
{
	MonoField* ScriptDropDownWindow::guiPanelField = nullptr;

	ScriptDropDownWindow::ScriptDropDownWindow(ManagedDropDownWindow* window)
		:ScriptObject(window->getManagedInstance()), mDropDownWindow(window)
	{
		mOnAssemblyRefreshStartedConn = ScriptObjectManager::instance().onRefreshStarted.connect(
			std::bind(&ScriptDropDownWindow::onAssemblyRefreshStarted, this));
	}

	ScriptDropDownWindow::~ScriptDropDownWindow()
	{
		mOnAssemblyRefreshStartedConn.disconnect();

		// Window must have been marked as deleted already
		assert(mDropDownWindow == nullptr);
	}

	void ScriptDropDownWindow::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptDropDownWindow::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Close", (void*)&ScriptDropDownWindow::internal_Close);
		metaData.scriptClass->addInternalCall("Internal_GetWidth", (void*)&ScriptDropDownWindow::internal_GetWidth);
		metaData.scriptClass->addInternalCall("Internal_SetWidth", (void*)&ScriptDropDownWindow::internal_SetWidth);
		metaData.scriptClass->addInternalCall("Internal_GetHeight", (void*)&ScriptDropDownWindow::internal_GetHeight);
		metaData.scriptClass->addInternalCall("Internal_SetHeight", (void*)&ScriptDropDownWindow::internal_SetHeight);
		metaData.scriptClass->addInternalCall("Internal_ScreenToWindowPos", (void*)&ScriptDropDownWindow::internal_ScreenToWindowPos);
		metaData.scriptClass->addInternalCall("Internal_WindowToScreenPos", (void*)&ScriptDropDownWindow::internal_WindowToScreenPos);

		guiPanelField = metaData.scriptClass->getField("GUI");
	}

	MonoObject* ScriptDropDownWindow::internal_CreateInstance(MonoString* ns, MonoString* typeName, 
		ScriptEditorWindow* parentWindow, Vector2I* position)
	{
		String strTypeName = toString(MonoUtil::monoToWString(typeName));
		String strNamespace = toString(MonoUtil::monoToWString(ns));
		String fullName = strNamespace + "." + strTypeName;

		MonoClass* windowClass = MonoManager::instance().findClass(strNamespace, strTypeName);
		if (windowClass == nullptr)
			return nullptr;

		MonoAssembly* assembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);

		MonoClass* defaultSizeAttrib = assembly->getClass("BansheeEditor", "DefaultSize");
		if (defaultSizeAttrib == nullptr)
			BS_EXCEPT(InternalErrorException, "Cannot find DefaultSize managed class.");

		MonoField* defaultWidthField = defaultSizeAttrib->getField("width");
		MonoField* defaultHeightField = defaultSizeAttrib->getField("height");

		int width = 200;
		int height = 200;

		MonoObject* defaultSizeObj = windowClass->getAttribute(defaultSizeAttrib);
		if (defaultSizeObj != nullptr)
		{
			defaultWidthField->get(defaultSizeObj, &width);
			defaultHeightField->get(defaultSizeObj, &height);
		}

		MonoObject* instance = windowClass->createInstance(false);

		ManagedDropDownWindow* dropDownWindow = nullptr;
		if (parentWindow != nullptr && !parentWindow->isDestroyed())
		{
			EditorWidgetBase* editorWidget = parentWindow->getEditorWidget();
			EditorWidgetContainer* parentContainer = editorWidget->_getParent();
			if (parentContainer != nullptr)
			{
				SPtr<RenderWindow> parentRenderWindow = parentContainer->getParentWindow()->getRenderWindow();
				SPtr<Camera> parentCamera = parentContainer->getParentWidget().getCamera();

				position->x += editorWidget->getX();
				position->y += editorWidget->getY();

				dropDownWindow = DropDownWindowManager::instance().open<ManagedDropDownWindow>(
					parentRenderWindow, parentCamera, *position, instance, width, height);
			}
		}

		ScriptDropDownWindow* nativeInstance = new (bs_alloc<ScriptDropDownWindow>()) ScriptDropDownWindow(dropDownWindow);

		if (dropDownWindow != nullptr)
			dropDownWindow->initialize(nativeInstance);

		windowClass->construct(instance);
		return instance;
	}

	void ScriptDropDownWindow::internal_Close(ScriptDropDownWindow* thisPtr)
	{
		if (thisPtr->mDropDownWindow != nullptr)
			DropDownWindowManager::instance().close();
	}

	void ScriptDropDownWindow::onAssemblyRefreshStarted()
	{
		if (mDropDownWindow != nullptr)
			DropDownWindowManager::instance().close();
	}

	void ScriptDropDownWindow::notifyWindowClosed()
	{
		mDropDownWindow = nullptr;
	}

	UINT32 ScriptDropDownWindow::internal_GetWidth(ScriptDropDownWindow* thisPtr)
	{
		if (thisPtr->mDropDownWindow != nullptr)
			return thisPtr->mDropDownWindow->getWidth();

		return 0;
	}

	void ScriptDropDownWindow::internal_SetWidth(ScriptDropDownWindow* thisPtr, UINT32 value)
	{
		if (thisPtr->mDropDownWindow != nullptr)
			thisPtr->mDropDownWindow->setSize(value, thisPtr->mDropDownWindow->getHeight());
	}

	UINT32 ScriptDropDownWindow::internal_GetHeight(ScriptDropDownWindow* thisPtr)
	{
		if (thisPtr->mDropDownWindow != nullptr)
			return thisPtr->mDropDownWindow->getHeight();

		return 0;
	}

	void ScriptDropDownWindow::internal_SetHeight(ScriptDropDownWindow* thisPtr, UINT32 value)
	{
		if (thisPtr->mDropDownWindow != nullptr)
			thisPtr->mDropDownWindow->setSize(thisPtr->mDropDownWindow->getWidth(), value);
	}

	void ScriptDropDownWindow::internal_ScreenToWindowPos(ScriptDropDownWindow* thisPtr, Vector2I* screenPos, Vector2I* windowPos)
	{
		if (thisPtr->mDropDownWindow != nullptr)
			*windowPos = thisPtr->mDropDownWindow->screenToWindowPos(*screenPos);
		else
			*windowPos = *screenPos;
	}

	void ScriptDropDownWindow::internal_WindowToScreenPos(ScriptDropDownWindow* thisPtr, Vector2I* windowPos, Vector2I* screenPos)
	{
		if (thisPtr->mDropDownWindow != nullptr)
			*screenPos = thisPtr->mDropDownWindow->windowToScreenPos(*windowPos);
		else
			*screenPos = *windowPos;
	}

	ManagedDropDownWindow::ManagedDropDownWindow(const SPtr<RenderWindow>& parent, const SPtr<Camera>& camera,
		const Vector2I& position, MonoObject* managedInstance, UINT32 width, UINT32 height)
		: DropDownWindow(parent, camera, position, width, height), mOnInitializeThunk(nullptr), mOnDestroyThunk(nullptr)
		, mUpdateThunk(nullptr), mIsInitialized(false), mManagedInstance(managedInstance), mGCHandle(0)
		, mScriptParent(nullptr), mContentsPanel(nullptr)
	{
		mGCHandle = MonoUtil::newGCHandle(mManagedInstance);
		mManagedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);

		MonoObject* guiPanel = ScriptGUIPanel::createFromExisting(mContents);
		mContentsPanel = ScriptGUILayout::toNative(guiPanel);
		ScriptDropDownWindow::guiPanelField->set(mManagedInstance, guiPanel);

		::MonoClass* rawMonoClass = MonoUtil::getClass(mManagedInstance);
		MonoClass* monoClass = MonoManager::instance().findClass(rawMonoClass);

		mNamespace = monoClass->getNamespace();
		mTypename = monoClass->getTypeName();

		reloadMonoTypes(monoClass);
	}

	ManagedDropDownWindow::~ManagedDropDownWindow()
	{
		mContentsPanel->destroy();
		mContentsPanel = nullptr;

		triggerOnDestroy();
		mScriptParent->notifyWindowClosed();

		MonoUtil::freeGCHandle(mGCHandle);
		mGCHandle = 0;
	}

	void ManagedDropDownWindow::initialize(ScriptDropDownWindow* parent)
	{
		mScriptParent = parent;
	}

	void ManagedDropDownWindow::triggerOnInitialize()
	{
		if (mOnInitializeThunk != nullptr && mManagedInstance != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnInitializeThunk, mManagedInstance);
		}
	}

	void ManagedDropDownWindow::triggerOnDestroy()
	{
		if (mOnDestroyThunk != nullptr && mManagedInstance != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDestroyThunk, mManagedInstance);
		}
	}

	void ManagedDropDownWindow::update()
	{
		if (!mIsInitialized)
		{
			triggerOnInitialize();
			mIsInitialized = true;
		}

		if (mUpdateThunk != nullptr && mManagedInstance != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mUpdateThunk, mManagedInstance);
		}
	}

	void ManagedDropDownWindow::reloadMonoTypes(MonoClass* windowClass)
	{
		MonoMethod* updateMethod = windowClass->getMethod("OnEditorUpdate", 0);

		if (updateMethod != nullptr)
			mUpdateThunk = (UpdateThunkDef)updateMethod->getThunk();

		MonoMethod* onInitializeMethod = windowClass->getMethod("OnInitialize", 0);

		if (onInitializeMethod != nullptr)
			mOnInitializeThunk = (OnInitializeThunkDef)onInitializeMethod->getThunk();

		MonoMethod* onDestroyMethod = windowClass->getMethod("OnDestroy", 0);

		if (onDestroyMethod != nullptr)
			mOnDestroyThunk = (OnDestroyThunkDef)onDestroyMethod->getThunk();
	}
}