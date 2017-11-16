//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptEditorWindow.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "EditorWindow/BsEditorWindow.h"
#include "EditorWindow/BsEditorWidget.h"
#include "EditorWindow/BsEditorWidgetManager.h"
#include "EditorWindow/BsEditorWidgetContainer.h"
#include "BsMonoAssembly.h"
#include "BsScriptObjectManager.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "Wrappers/BsScriptHString.h"
#include "Platform/BsPlatform.h"
#include "Input/BsInput.h"
#include "Wrappers/BsScriptUndoRedo.h"

using namespace std::placeholders;

namespace bs
{
	UnorderedMap<String, ScriptEditorWindow*> ScriptEditorWindow::OpenScriptEditorWindows;
	Vector<String> ScriptEditorWindow::AvailableWindowTypes;
	MonoMethod* ScriptEditorWindow::onResizedMethod = nullptr;
	MonoMethod* ScriptEditorWindow::onFocusChangedMethod = nullptr;
	MonoField* ScriptEditorWindow::guiPanelField = nullptr;
	MonoField* ScriptEditorWindow::undoRedoField = nullptr;

	ScriptEditorWindow::ScriptEditorWindow(ScriptEditorWidget* editorWidget)
		: ScriptObject(editorWidget->getManagedInstance()), mName(editorWidget->getName())
		, mEditorWidget(editorWidget), mRefreshInProgress(false), mIsDestroyed(false)
	{
		mOnWidgetResizedConn = editorWidget->onResized.connect(std::bind(&ScriptEditorWindow::onWidgetResized, this, _1, _2));
		mOnFocusChangedConn = editorWidget->onFocusChanged.connect(std::bind(&ScriptEditorWindow::onFocusChanged, this, _1));

		mOnAssemblyRefreshStartedConn = ScriptObjectManager::instance().onRefreshStarted.connect(
			std::bind(&ScriptEditorWindow::onAssemblyRefreshStarted, this));
	}

	ScriptEditorWindow::~ScriptEditorWindow()
	{
		// TODO - This probably need to be called whenever we close a window, and not when script class is destructed
		mOnWidgetResizedConn.disconnect();
		mOnFocusChangedConn.disconnect();

		mOnAssemblyRefreshStartedConn.disconnect();
	}

	void ScriptEditorWindow::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateOrGetInstance", (void*)&ScriptEditorWindow::internal_createOrGetInstance);
		metaData.scriptClass->addInternalCall("Internal_GetInstance", (void*)&ScriptEditorWindow::internal_getInstance);
		metaData.scriptClass->addInternalCall("Internal_GetAllWindows", (void*)&ScriptEditorWindow::internal_getAllWindows);

		metaData.scriptClass->addInternalCall("Internal_GetWidth", (void*)&ScriptEditorWindow::internal_getWidth);
		metaData.scriptClass->addInternalCall("Internal_GetHeight", (void*)&ScriptEditorWindow::internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_GetBounds", (void*)&ScriptEditorWindow::internal_getBounds);
		metaData.scriptClass->addInternalCall("Internal_SetFocus", (void*)&ScriptEditorWindow::internal_setFocus);
		metaData.scriptClass->addInternalCall("Internal_HasFocus", (void*)&ScriptEditorWindow::internal_hasFocus);
		metaData.scriptClass->addInternalCall("Internal_SetActive", (void*)&ScriptEditorWindow::internal_setActive);
		metaData.scriptClass->addInternalCall("Internal_IsActive", (void*)&ScriptEditorWindow::internal_isActive);
		metaData.scriptClass->addInternalCall("Internal_IsPointerHovering", (void*)&ScriptEditorWindow::internal_isPointerHovering);
		metaData.scriptClass->addInternalCall("Internal_ScreenToWindowPos", (void*)&ScriptEditorWindow::internal_screenToWindowPos);
		metaData.scriptClass->addInternalCall("Internal_WindowToScreenPos", (void*)&ScriptEditorWindow::internal_windowToScreenPos);

		onResizedMethod = metaData.scriptClass->getMethod("WindowResized", 2);
		onFocusChangedMethod = metaData.scriptClass->getMethod("FocusChanged", 1);

		guiPanelField = metaData.scriptClass->getField("GUI");
		undoRedoField = metaData.scriptClass->getField("undoRedo");
	}

	MonoObject* ScriptEditorWindow::internal_createOrGetInstance(MonoString* ns, MonoString* typeName)
	{
		String strTypeName = toString(MonoUtil::monoToWString(typeName));
		String strNamespace = toString(MonoUtil::monoToWString(ns));
		String fullName = strNamespace + "." + strTypeName;

		auto findIter = OpenScriptEditorWindows.find(fullName);
		if(findIter != OpenScriptEditorWindows.end())
			return findIter->second->mEditorWidget->getManagedInstance();

		EditorWidgetManager::instance().open(fullName); // This will trigger openEditorWidgetCallback and update OpenScriptEditorWindows

		auto findIter2 = OpenScriptEditorWindows.find(fullName);
		if(findIter2 != OpenScriptEditorWindows.end())
			return findIter2->second->mEditorWidget->getManagedInstance();

		return nullptr;
	}

	MonoObject* ScriptEditorWindow::internal_getInstance(MonoString* ns, MonoString* typeName)
	{
		String strTypeName = toString(MonoUtil::monoToWString(typeName));
		String strNamespace = toString(MonoUtil::monoToWString(ns));
		String fullName = strNamespace + "." + strTypeName;

		auto findIter = OpenScriptEditorWindows.find(fullName);
		if (findIter != OpenScriptEditorWindows.end())
			return findIter->second->mEditorWidget->getManagedInstance();

		return nullptr;
	}

	MonoArray* ScriptEditorWindow::internal_getAllWindows()
	{
		UINT32 numWindows = (UINT32)OpenScriptEditorWindows.size();
		ScriptArray output = ScriptArray::create<ScriptEditorWindow>(numWindows);

		UINT32 idx = 0;
		for (auto& entry : OpenScriptEditorWindows)
			output.set(idx++, entry.second->mEditorWidget->getManagedInstance());

		return output.getInternal();
	}

	EditorWidgetBase* ScriptEditorWindow::getEditorWidget() const 
	{ 
		return mEditorWidget; 
	}

	void ScriptEditorWindow::_onManagedInstanceDeleted()
	{
		if (!mRefreshInProgress)
			ScriptObject::_onManagedInstanceDeleted();
		else
			mEditorWidget->clearManagedInstance(false);
	}

	ScriptObjectBackup ScriptEditorWindow::beginRefresh()
	{
		mRefreshInProgress = true;

		auto iterFind = OpenScriptEditorWindows.find(mName);
		if (iterFind != OpenScriptEditorWindows.end())
			iterFind->second->mEditorWidget->clearManagedInstance(true);

		return PersistentScriptObjectBase::beginRefresh();
	}

	void ScriptEditorWindow::endRefresh(const ScriptObjectBackup& backupData)
	{
		mRefreshInProgress = false;

		if (isDestroyed())
		{
			// We couldn't restore managed instance because window class cannot be found
			_onManagedInstanceDeleted();
		}

		PersistentScriptObjectBase::endRefresh(backupData);
	}

	void ScriptEditorWindow::onAssemblyRefreshStarted()
	{
		if (!isDestroyed())
			mEditorWidget->triggerOnDestroy();
	}

	MonoObject* ScriptEditorWindow::_createManagedInstance(bool construct)
	{
		if (!isDestroyed())
		{
			mEditorWidget->createManagedInstance();

			return mEditorWidget->getManagedInstance();
		}
		else
			return nullptr;
	}

	void ScriptEditorWindow::internal_setFocus(ScriptEditorWindow* thisPtr, bool focus)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->getEditorWidget()->setHasFocus(focus);
	}

	bool ScriptEditorWindow::internal_hasFocus(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->getEditorWidget()->hasFocus();
		else
			return false;
	}

	void ScriptEditorWindow::internal_setActive(ScriptEditorWindow* thisPtr, bool active)
	{
		if (!thisPtr->isDestroyed())
		{
			if (active)
				thisPtr->getEditorWidget()->setActive(); 
		}
	}

	bool ScriptEditorWindow::internal_isActive(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->getEditorWidget()->isActive();
		else
			return false;
	}

	bool ScriptEditorWindow::internal_isPointerHovering(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
		{
			EditorWidgetBase* widget = thisPtr->getEditorWidget();
			EditorWindowBase* window = widget->getParentWindow();
			if (window == nullptr)
				return false;

			SPtr<RenderWindow> renderWindow = window->getRenderWindow();

			Vector2I pointerPos = gInput().getPointerPosition();
			if(Platform::isPointOverWindow(*renderWindow, pointerPos))
			{
				Rect2I bounds = thisPtr->getEditorWidget()->getBounds();
				Vector2I screenPos = widget->widgetToScreenPos(Vector2I(0, 0));

				bounds.x = screenPos.x;
				bounds.y = screenPos.y;

				return bounds.contains(pointerPos);
			}
		}

		return false;
	}

	void ScriptEditorWindow::internal_screenToWindowPos(ScriptEditorWindow* thisPtr, Vector2I* screenPos, Vector2I* windowPos)
	{
		if (!thisPtr->isDestroyed())
			*windowPos = thisPtr->getEditorWidget()->screenToWidgetPos(*screenPos);
		else
			*windowPos = *screenPos;
	}

	void ScriptEditorWindow::internal_windowToScreenPos(ScriptEditorWindow* thisPtr, Vector2I* windowPos, Vector2I* screenPos)
	{
		if (!thisPtr->isDestroyed())
			*screenPos = thisPtr->getEditorWidget()->widgetToScreenPos(*windowPos);
		else
			*screenPos = *windowPos;
	}

	UINT32 ScriptEditorWindow::internal_getWidth(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->mEditorWidget->getWidth();
		else
			return 0;
	}

	UINT32 ScriptEditorWindow::internal_getHeight(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->mEditorWidget->getHeight();
		else
			return 0;
	}

	void ScriptEditorWindow::internal_getBounds(ScriptEditorWindow* thisPtr, Rect2I* bounds)
	{
		if (!thisPtr->isDestroyed())
		{
			EditorWidgetBase* widget = thisPtr->getEditorWidget();
			*bounds = thisPtr->getEditorWidget()->getBounds();
			
			Vector2I widgetPos(0, 0);
			Vector2I screenPos = widget->widgetToScreenPos(widgetPos);

			bounds->x = screenPos.x;
			bounds->y = screenPos.y;
		}
		else
			*bounds = Rect2I();
	}

	void ScriptEditorWindow::onWidgetResized(UINT32 width, UINT32 height)
	{
		MonoObject* managedInstance = mEditorWidget->getManagedInstance();
		if (isDestroyed() || !mEditorWidget->isInitialized() || managedInstance == nullptr)
			return;

		void* params[2] = { &width, &height };
		onResizedMethod->invokeVirtual(managedInstance, params);
	}

	void ScriptEditorWindow::onFocusChanged(bool inFocus)
	{
		MonoObject* managedInstance = mEditorWidget->getManagedInstance();
		if (isDestroyed() || managedInstance == nullptr)
			return;

		void* params[1] = { &inFocus};
		onFocusChangedMethod->invokeVirtual(managedInstance, params);
	}

	void ScriptEditorWindow::registerManagedEditorWindows()
	{
		MonoAssembly* assembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);

		if(assembly != nullptr)
		{
			MonoClass* defaultSizeAttrib = assembly->getClass("BansheeEditor", "DefaultSize");
			if (defaultSizeAttrib == nullptr)
				BS_EXCEPT(InternalErrorException, "Cannot find DefaultSize managed attribute.");

			MonoField* defaultWidthField = defaultSizeAttrib->getField("width");
			MonoField* defaultHeightField = defaultSizeAttrib->getField("height");

			MonoClass* undoRedoLocalAttrib = assembly->getClass("BansheeEditor", "UndoRedoLocal");
			if (undoRedoLocalAttrib == nullptr)
				BS_EXCEPT(InternalErrorException, "Cannot find UndoRedoLocal managed attribute.");

			MonoClass* editorWindowClass = assembly->getClass("BansheeEditor", "EditorWindow");

			const Vector<MonoClass*>& allClasses = assembly->getAllClasses();
			for(auto& curClass : allClasses)
			{
				if(curClass->isSubClassOf(editorWindowClass) && curClass != editorWindowClass)
				{
					UINT32 width = 400;
					UINT32 height = 400;

					MonoObject* defaultSize = curClass->getAttribute(defaultSizeAttrib);
					if (defaultSize != nullptr)
					{
						defaultWidthField->get(defaultSize, &width);
						defaultHeightField->get(defaultSize, &height);
					}

					bool hasLocalUndoRedo = curClass->getAttribute(undoRedoLocalAttrib) != nullptr;

					const String& className = curClass->getFullName();
					EditorWidgetManager::instance().registerWidget(className, 
						std::bind(&ScriptEditorWindow::openEditorWidgetCallback, curClass->getNamespace(), 
						curClass->getTypeName(), width, height, hasLocalUndoRedo, _1));
					AvailableWindowTypes.push_back(className);
				}
			}
		}
	}

	void ScriptEditorWindow::clearRegisteredEditorWindow()
	{
		for (auto& windowType : AvailableWindowTypes)
		{
			EditorWidgetManager::instance().unregisterWidget(windowType);
		}

		AvailableWindowTypes.clear();
	}

	EditorWidgetBase* ScriptEditorWindow::openEditorWidgetCallback(String ns, String type, UINT32 width, 
		UINT32 height, bool localUndoRedo, EditorWidgetContainer& parentContainer)
	{
		ScriptEditorWidget* editorWidget = bs_new<ScriptEditorWidget>(ns, type, width, height, localUndoRedo, parentContainer);
		ScriptEditorWindow* nativeInstance = new (bs_alloc<ScriptEditorWindow>()) ScriptEditorWindow(editorWidget);

		ScriptEditorWindow::registerScriptEditorWindow(nativeInstance);

		MonoClass::construct(editorWidget->getManagedInstance());
		editorWidget->setScriptOwner(nativeInstance);

		return editorWidget;
	}

	void ScriptEditorWindow::registerScriptEditorWindow(ScriptEditorWindow* editorWindow)
	{
		if(editorWindow == nullptr)
			return;

		auto findIter = OpenScriptEditorWindows.find(editorWindow->mName);
		if(findIter == OpenScriptEditorWindows.end())
			OpenScriptEditorWindows[editorWindow->mName] = editorWindow;
	}

	void ScriptEditorWindow::unregisterScriptEditorWindow(const String& windowTypeName)
	{
		auto findIter = OpenScriptEditorWindows.find(windowTypeName);
		if(findIter != OpenScriptEditorWindows.end())
			OpenScriptEditorWindows.erase(findIter);
	}

	ScriptEditorWidget::ScriptEditorWidget(const String& ns, const String& type, UINT32 defaultWidth, 
		UINT32 defaultHeight, bool localUndoRedo, EditorWidgetContainer& parentContainer)
		: EditorWidgetBase(HString(toWString(type)), ns + "." + type, defaultWidth, defaultHeight, parentContainer)
		, mNamespace(ns), mTypename(type), mOnInitializeThunk(nullptr), mOnDestroyThunk(nullptr), mUpdateThunk(nullptr)
		, mManagedInstance(nullptr), mGCHandle(0), mGetDisplayName(nullptr), mScriptOwner(nullptr), mContentsPanel(nullptr)
		, mIsInitialized(false), mHasLocalUndoRedo(localUndoRedo)
	{
		if(createManagedInstance())
		{
			if (mGetDisplayName != nullptr)
			{
				MonoObject* displayNameMono = mGetDisplayName->invokeVirtual(mManagedInstance, nullptr);
				ScriptHString* scriptHString = ScriptHString::toNative(displayNameMono);

				if (scriptHString != nullptr)
				{
					mDisplayName = HString(scriptHString->getInternalValue());
					parentContainer.refreshWidgetNames();
				}
			}
		}
	}

	ScriptEditorWidget::~ScriptEditorWidget()
	{
		mScriptOwner->mIsDestroyed = true;
		mScriptOwner->mEditorWidget = nullptr;

		mContentsPanel->destroy();
		mContentsPanel = nullptr;

		triggerOnDestroy();

		clearManagedInstance(true);

		ScriptEditorWindow::unregisterScriptEditorWindow(getName());
	}

	void ScriptEditorWidget::clearManagedInstance(bool freeHandle)
	{
		if(freeHandle)
			MonoUtil::freeGCHandle(mGCHandle);

		mManagedInstance = nullptr;
		mGCHandle = 0;
	}

	bool ScriptEditorWidget::createManagedInstance()
	{
		const char* assemblies[2] = { EDITOR_ASSEMBLY, SCRIPT_EDITOR_ASSEMBLY };
		UINT32 numAssemblies = sizeof(assemblies) / sizeof(assemblies[0]);

		for (UINT32 i = 0; i < numAssemblies; i++)
		{
			MonoAssembly* assembly = MonoManager::instance().getAssembly(assemblies[i]);

			if (assembly != nullptr)
			{
				MonoClass* editorWindowClass = assembly->getClass(mNamespace, mTypename);

				if (editorWindowClass != nullptr)
				{
					mManagedInstance = editorWindowClass->createInstance();
					mGCHandle = MonoUtil::newGCHandle(mManagedInstance);

					MonoObject* guiPanel = ScriptGUIPanel::createFromExisting(mContent);
					mContentsPanel = ScriptGUILayout::toNative(guiPanel);
					ScriptEditorWindow::guiPanelField->set(mManagedInstance, guiPanel);

					if(mHasLocalUndoRedo)
					{
						MonoObject* undoRedo = ScriptUndoRedo::create();
						ScriptEditorWindow::undoRedoField->set(mManagedInstance, undoRedo);
					}

					reloadMonoTypes(editorWindowClass);
					return true;
				}
			}
		}

		return false;
	}

	void ScriptEditorWidget::triggerOnInitialize()
	{
		if (mOnInitializeThunk != nullptr && mManagedInstance != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnInitializeThunk, mManagedInstance);
		}

		mIsInitialized = true;
	}

	void ScriptEditorWidget::triggerOnDestroy()
	{
		if (mOnDestroyThunk != nullptr && mManagedInstance != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mOnDestroyThunk, mManagedInstance);
		}

		mIsInitialized = false;
	}

	void ScriptEditorWidget::update()
	{
		if (!mIsInitialized)
			triggerOnInitialize();

		if (mUpdateThunk != nullptr && mManagedInstance != nullptr)
		{
			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			MonoUtil::invokeThunk(mUpdateThunk, mManagedInstance);
		}
	}

	void ScriptEditorWidget::reloadMonoTypes(MonoClass* windowClass)
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

		mGetDisplayName = windowClass->getMethod("GetDisplayName", 0);
	}
}