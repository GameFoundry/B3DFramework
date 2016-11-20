//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMenuItemManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoField.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptObjectManager.h"
#include "BsShortcutKey.h"
#include "BsEditorWindowManager.h"
#include "BsMainEditorWindow.h"
#include "BsGUIMenuBar.h"
#include "BsGUIMenu.h"

using namespace std::placeholders;

namespace bs
{
	MenuItemManager::MenuItemManager(ScriptAssemblyManager& scriptObjectManager)
		:mScriptObjectManager(scriptObjectManager), mMenuItemAttribute(nullptr), mPathField(nullptr), 
		mShortcutField(nullptr), mPriorityField(nullptr), mSeparatorField(nullptr)
	{
		mDomainLoadedConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(std::bind(&MenuItemManager::reloadAssemblyData, this));
		reloadAssemblyData();
	}

	MenuItemManager::~MenuItemManager()
	{
		mDomainLoadedConn.disconnect();
	}

	void MenuItemManager::clearMenuItems()
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();

		UINT32 numItems = (UINT32)mMenuItems.size();
		Vector<bool> destroyedItems(numItems, false);

		// Remove leaf elements only to avoid deleting child menu items
		bool changesMade;
		do
		{
			changesMade = false;
			for (UINT32 i = 0; i < numItems; i++)
			{
				GUIMenuItem* menuItem = mMenuItems[i];
				if (!destroyedItems[i] && menuItem->getNumChildren() == 0)
				{
					mainWindow->getMenuBar().removeMenuItem(menuItem);

					destroyedItems[i] = true;
					changesMade = true;
				}
			}
		} while (changesMade);

		// Remove remaining items regardless (none of their children are our concern). But when running properly there 
		// should be no entries to remove at this step.
		for (UINT32 i = 0; i < numItems; i++)
		{
			GUIMenuItem* menuItem = mMenuItems[i];
			if (!destroyedItems[i])
				mainWindow->getMenuBar().removeMenuItem(menuItem);
		}

		mMenuItems.clear();
	}

	void MenuItemManager::reloadAssemblyData()
	{
		clearMenuItems();

		// Reload MenuItem attribute from editor assembly
		MonoAssembly* editorAssembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);
		mMenuItemAttribute = editorAssembly->getClass("BansheeEditor", "MenuItem");
		if (mMenuItemAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find MenuItem managed class.");

		mPathField = mMenuItemAttribute->getField("path");
		mShortcutField = mMenuItemAttribute->getField("shortcut");
		mPriorityField = mMenuItemAttribute->getField("priority");
		mSeparatorField = mMenuItemAttribute->getField("separator");

		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();

		Vector<String> scriptAssemblyNames = mScriptObjectManager.getScriptAssemblies();
		for (auto& assemblyName : scriptAssemblyNames)
		{
			MonoAssembly* assembly = MonoManager::instance().getAssembly(assemblyName);

			// Find new menu item methods
			const Vector<MonoClass*>& allClasses = assembly->getAllClasses();
			for (auto curClass : allClasses)
			{
				const Vector<MonoMethod*>& methods = curClass->getAllMethods();
				for (auto& curMethod : methods)
				{
					WString path;
					ShortcutKey shortcutKey = ShortcutKey::NONE;
					INT32 priority = 0;
					bool separator = false;
					if (parseMenuItemMethod(curMethod, path, shortcutKey, priority, separator))
					{
						std::function<void()> callback = std::bind(&MenuItemManager::menuItemCallback, curMethod);

						if (separator)
						{
							Vector<WString> pathElements = StringUtil::split(path, L"/");
							WString separatorPath;
							if (pathElements.size() > 1)
							{
								const WString& lastElem = pathElements[pathElements.size() - 1];
								separatorPath = path;
								separatorPath.erase(path.size() - lastElem.size() - 1, lastElem.size() + 1);
							}

							GUIMenuItem* separatorItem = mainWindow->getMenuBar().addMenuItemSeparator(separatorPath, priority);
							mMenuItems.push_back(separatorItem);
						}

						GUIMenuItem* menuItem = mainWindow->getMenuBar().addMenuItem(path, callback, priority, shortcutKey);
						mMenuItems.push_back(menuItem);
					}
				}
			}
		}
	}

	bool MenuItemManager::parseMenuItemMethod(MonoMethod* method, WString& path, ShortcutKey& shortcut, INT32& priority, bool& separator) const
	{
		if (!method->hasAttribute(mMenuItemAttribute))
			return false;

		if (method->getNumParameters() != 0)
			return false;

		if (!method->isStatic())
			return false;

		MonoObject* menuItemAttrib = method->getAttribute(mMenuItemAttribute);

		MonoString* monoPath;
		mPathField->getValue(menuItemAttrib, &monoPath);

		mShortcutField->getValue(menuItemAttrib, &shortcut);
		path = MonoUtil::monoToWString(monoPath);
		mPriorityField->getValue(menuItemAttrib, &priority);
		mSeparatorField->getValue(menuItemAttrib, &separator);

		return true;
	}

	void MenuItemManager::menuItemCallback(MonoMethod* method)
	{
		method->invoke(nullptr, nullptr);
	}
}