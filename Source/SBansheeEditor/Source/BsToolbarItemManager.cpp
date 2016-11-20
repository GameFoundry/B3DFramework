//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsToolbarItemManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoField.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptObjectManager.h"
#include "BsEditorWindowManager.h"
#include "BsMainEditorWindow.h"
#include "BsGUIMenuBar.h"
#include "BsGUIContent.h"
#include "BsBuiltinEditorResources.h"
#include "BsProjectLibrary.h"

using namespace std::placeholders;

namespace bs
{
	ToolbarItemManager::ToolbarItemManager(ScriptAssemblyManager& scriptObjectManager)
		:mScriptObjectManager(scriptObjectManager), mToolbarItemAttribute(nullptr), mNameField(nullptr)
		, mIconField(nullptr), mBuiltinIconField(nullptr), mTooltipField(nullptr), mPriorityField(nullptr)
		, mSeparatorField(nullptr)
		
	{
		mDomainLoadedConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(std::bind(&ToolbarItemManager::reloadAssemblyData, this));
		reloadAssemblyData();
	}

	ToolbarItemManager::~ToolbarItemManager()
	{
		mDomainLoadedConn.disconnect();
	}

	void ToolbarItemManager::clearToolbarItems()
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();

		for (auto& toolbarItem : mToolbarItems)
			mainWindow->getMenuBar().removeToolBarButton(toolbarItem);
	}

	void ToolbarItemManager::reloadAssemblyData()
	{
		clearToolbarItems();

		// Reload MenuItem attribute from editor assembly
		MonoAssembly* editorAssembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);
		mToolbarItemAttribute = editorAssembly->getClass("BansheeEditor", "ToolbarItem");
		if (mToolbarItemAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find ToolbarItem managed class.");

		mNameField = mToolbarItemAttribute->getField("name");
		mIconField = mToolbarItemAttribute->getField("icon");
		mBuiltinIconField = mToolbarItemAttribute->getField("builtinIcon");
		mTooltipField = mToolbarItemAttribute->getField("tooltip");
		mPriorityField = mToolbarItemAttribute->getField("priority");
		mSeparatorField = mToolbarItemAttribute->getField("separator");

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
					String name;
					HSpriteTexture icon;
					HString tooltip;
					INT32 priority = 0;
					bool separator = false;
					if (parseToolbarItemMethod(curMethod, name, icon, tooltip, priority, separator))
					{
						std::function<void()> callback = std::bind(&ToolbarItemManager::toolbarItemCallback, curMethod);

						if (separator)
						{
							String sepName = "s__" + name;

							mainWindow->getMenuBar().addToolBarSeparator(sepName, priority);
							mToolbarItems.push_back(sepName);
						}

						GUIContent content(icon, tooltip);
						mainWindow->getMenuBar().addToolBarButton(name, content, callback, priority);
						mToolbarItems.push_back(name);
					}
				}
			}
		}
	}

	bool ToolbarItemManager::parseToolbarItemMethod(MonoMethod* method, String& name, HSpriteTexture& icon, HString& tooltip,
		INT32& priority, bool& separator) const
	{
		if (!method->hasAttribute(mToolbarItemAttribute))
			return false;

		if (method->getNumParameters() != 0)
			return false;

		if (!method->isStatic())
			return false;

		MonoObject* toolbarItemAttrib = method->getAttribute(mToolbarItemAttribute);

		MonoString* monoName;
		mNameField->getValue(toolbarItemAttrib, &monoName);
		name = MonoUtil::monoToString(monoName);

		int builtinIcon;
		mBuiltinIconField->getValue(toolbarItemAttrib, &builtinIcon);

		if (builtinIcon != -1)
		{
			icon = BuiltinEditorResources::instance().getToolbarIcon((ToolbarIcon)builtinIcon);
		}
		else
		{
			MonoString* monoTexturePath;
			mIconField->getValue(toolbarItemAttrib, &monoTexturePath);

			Path texturePath = MonoUtil::monoToWString(monoTexturePath);
			icon = static_resource_cast<SpriteTexture>(gProjectLibrary().load(texturePath));
		}

		MonoString* tooltipMono;
		mTooltipField->getValue(toolbarItemAttrib, &tooltipMono);

		tooltip = HString(MonoUtil::monoToWString(tooltipMono));

		mPriorityField->getValue(toolbarItemAttrib, &priority);
		mSeparatorField->getValue(toolbarItemAttrib, &separator);

		return true;
	}

	void ToolbarItemManager::toolbarItemCallback(MonoMethod* method)
	{
		method->invoke(nullptr, nullptr);
	}
}