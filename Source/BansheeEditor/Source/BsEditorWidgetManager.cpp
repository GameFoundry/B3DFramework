//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorWidgetManager.h"
#include "BsEditorWidget.h"
#include "BsEditorWindow.h"
#include "BsEditorWidgetContainer.h"
#include "BsEditorWindowManager.h"
#include "BsMainEditorWindow.h"
#include "BsEditorWidgetLayout.h"
#include "BsDockManager.h"
#include "BsException.h"
#include "BsInput.h"
#include "BsRenderWindow.h"
#include "BsRenderWindowManager.h"
#include "BsVector2I.h"
#include "BsCoreThread.h"

using namespace std::placeholders;

namespace bs
{
	Stack<std::pair<String, std::function<EditorWidgetBase*(EditorWidgetContainer&)>>> EditorWidgetManager::QueuedCreateCallbacks;

	EditorWidgetManager::EditorWidgetManager()
	{
		while(!QueuedCreateCallbacks.empty())
		{
			std::pair<String, std::function<EditorWidgetBase*(EditorWidgetContainer&)>> curElement = QueuedCreateCallbacks.top();
			QueuedCreateCallbacks.pop();

			registerWidget(curElement.first, curElement.second);
		}

		mOnFocusLostConn = RenderWindowManager::instance().onFocusLost.connect(std::bind(&EditorWidgetManager::onFocusLost, this, _1));
		mOnFocusGainedConn = RenderWindowManager::instance().onFocusGained.connect(std::bind(&EditorWidgetManager::onFocusGained, this, _1));
	}

	EditorWidgetManager::~EditorWidgetManager()
	{
		mOnFocusLostConn.disconnect();
		mOnFocusGainedConn.disconnect();

		Map<String, EditorWidgetBase*> widgetsCopy = mActiveWidgets;

		for (auto& widget : widgetsCopy)
			widget.second->close();
	}

	void EditorWidgetManager::update()
	{
		if (gInput().isPointerButtonDown(PointerEventButton::Left) || gInput().isPointerButtonDown(PointerEventButton::Right))
		{
			for (auto& widgetData : mActiveWidgets)
			{
				EditorWidgetBase* widget = widgetData.second;
				EditorWidgetContainer* parentContainer = widget->_getParent();
				if (parentContainer == nullptr)
				{
					widget->_setHasFocus(false);
					continue;
				}

				EditorWindowBase* parentWindow = parentContainer->getParentWindow();
				SPtr<RenderWindow> parentRenderWindow = parentWindow->getRenderWindow();
				const RenderWindowProperties& props = parentRenderWindow->getProperties();

				if (!props.hasFocus())
				{
					widget->_setHasFocus(false);
					continue;
				}

				if (parentContainer->getActiveWidget() != widget)
				{
					widget->_setHasFocus(false);
					continue;
				}

				Vector2I widgetPos = widget->screenToWidgetPos(gInput().getPointerPosition());
				if (widgetPos.x >= 0 && widgetPos.y >= 0
					&& widgetPos.x < (INT32)widget->getWidth()
					&& widgetPos.y < (INT32)widget->getHeight())
				{
					widget->_setHasFocus(true);
				}
				else
					widget->_setHasFocus(false);
			}
		}
	}

	void EditorWidgetManager::registerWidget(const String& name, std::function<EditorWidgetBase*(EditorWidgetContainer&)> createCallback)
	{
		auto iterFind = mCreateCallbacks.find(name);

		if(iterFind != mCreateCallbacks.end())
			BS_EXCEPT(InvalidParametersException, "Widget with the same name is already registered. Name: \"" + name + "\"");

		mCreateCallbacks[name] = createCallback;
	}

	void EditorWidgetManager::unregisterWidget(const String& name)
	{
		mCreateCallbacks.erase(name);
	}

	EditorWidgetBase* EditorWidgetManager::open(const String& name)
	{
		auto iterFind = mActiveWidgets.find(name);

		if(iterFind != mActiveWidgets.end())
			return iterFind->second;

		EditorWindow* window = EditorWindow::create();
		EditorWidgetBase* newWidget = create(name, window->widgets());
		if(newWidget == nullptr)
		{
			window->close();
			return nullptr;
		}

		Vector2I widgetSize(newWidget->getDefaultWidth(), newWidget->getDefaultHeight());
		Vector2I windowSize = EditorWidgetContainer::widgetToWindowSize(widgetSize);
		window->setSize((UINT32)windowSize.x, (UINT32)windowSize.y);

		return newWidget;
	}

	void EditorWidgetManager::close(EditorWidgetBase* widget)
	{
		auto findIter = std::find_if(mActiveWidgets.begin(), mActiveWidgets.end(),
			[&] (const std::pair<String, EditorWidgetBase*>& entry) { return entry.second == widget; });

		if(findIter != mActiveWidgets.end())
			mActiveWidgets.erase(findIter);

		if(widget->mParent != nullptr)
			widget->mParent->_notifyWidgetDestroyed(widget);

		EditorWidgetBase::destroy(widget);
	}

	void EditorWidgetManager::closeAll()
	{
		Vector<EditorWidgetBase*> toClose(mActiveWidgets.size());

		UINT32 idx = 0;
		for (auto& widget : mActiveWidgets)
			toClose[idx++] = widget.second;

		for (auto& widget : toClose)
			widget->close();
	}

	EditorWidgetBase* EditorWidgetManager::create(const String& name, EditorWidgetContainer& parentContainer)
	{
		auto iterFind = mActiveWidgets.find(name);

		if(iterFind != mActiveWidgets.end())
		{
			EditorWidgetBase* existingWidget = iterFind->second;
			if(existingWidget->_getParent() != nullptr && existingWidget->_getParent() != &parentContainer)
				existingWidget->_getParent()->remove(*existingWidget);

			if(existingWidget->_getParent() != &parentContainer)
				parentContainer.add(*iterFind->second);

			return iterFind->second;
		}

		auto iterFindCreate = mCreateCallbacks.find(name);
		if(iterFindCreate == mCreateCallbacks.end())
			return nullptr;

		EditorWidgetBase* newWidget = mCreateCallbacks[name](parentContainer);
		parentContainer.add(*newWidget);

		if(newWidget != nullptr)
			mActiveWidgets[name] = newWidget;

		return newWidget;
	}

	bool EditorWidgetManager::isValidWidget(const String& name) const
	{
		auto iterFindCreate = mCreateCallbacks.find(name);
		return iterFindCreate != mCreateCallbacks.end();
	}

	SPtr<EditorWidgetLayout> EditorWidgetManager::getLayout() const
	{
		auto GetWidgetNamesInContainer = [&] (const EditorWidgetContainer* container)
		{
			Vector<String> widgetNames;
			if(container != nullptr)
			{
				UINT32 numWidgets = container->getNumWidgets();

				for(UINT32 i = 0; i < numWidgets; i++)
				{
					EditorWidgetBase* widget = container->getWidget(i);
					widgetNames.push_back(widget->getName());
				}				
			}

			return widgetNames;
		};

		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		DockManager& dockManager = mainWindow->getDockManager();
		SPtr<EditorWidgetLayout> layout = bs_shared_ptr_new<EditorWidgetLayout>(dockManager.getLayout());

		Vector<EditorWidgetLayout::Entry>& layoutEntries = layout->getEntries();
		UnorderedSet<EditorWidgetContainer*> widgetContainers;

		for(auto& widget : mActiveWidgets)
		{
			widgetContainers.insert(widget.second->_getParent());
		}

		for(auto& widgetContainer : widgetContainers)
		{
			if(widgetContainer == nullptr)
				continue;

			layoutEntries.push_back(EditorWidgetLayout::Entry());
			EditorWidgetLayout::Entry& entry = layoutEntries.back();

			entry.widgetNames = GetWidgetNamesInContainer(widgetContainer);

			EditorWindowBase* parentWindow = widgetContainer->getParentWindow();
			entry.isDocked = parentWindow->isMain(); // Assumed widget is docked if part of main window
			
			if(!entry.isDocked)
			{
				entry.x = parentWindow->getLeft();
				entry.y = parentWindow->getTop();
				entry.width = parentWindow->getWidth();
				entry.height = parentWindow->getHeight();
			}
		}

		layout->setIsMainWindowMaximized(mainWindow->getRenderWindow()->getProperties().isMaximized());

		return layout;
	}

	void EditorWidgetManager::setLayout(const SPtr<EditorWidgetLayout>& layout)
	{
		// Unparent all widgets
		Vector<EditorWidgetBase*> unparentedWidgets;
		for(auto& widget : mActiveWidgets)
		{
			if(widget.second->_getParent() != nullptr)
				widget.second->_getParent()->remove(*(widget.second));

			unparentedWidgets.push_back(widget.second);
		}

		// Restore floating widgets
		for(auto& entry : layout->getEntries())
		{
			if(entry.isDocked)
				continue;

			EditorWindow* window = EditorWindow::create();
			for(auto& widgetName : entry.widgetNames)
			{
				create(widgetName, window->widgets());
			}

			window->setPosition(entry.x, entry.y);
			window->setSize(entry.width, entry.height);

			if(window->widgets().getNumWidgets() == 0)
				window->close();
		}

		// Restore docked widgets
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		DockManager& dockManager = mainWindow->getDockManager();

		dockManager.setLayout(layout->getDockLayout());

		// Destroy any widgets that are no longer have parents
		for(auto& widget : unparentedWidgets)
		{
			if(widget->_getParent() == nullptr)
				widget->close();
		}

		if (layout->getIsMainWindowMaximized())
			mainWindow->getRenderWindow()->maximize(gCoreAccessor());
	}

	void EditorWidgetManager::onFocusGained(const RenderWindow& window)
	{
		// Do nothing, possibly regain focus on last focused widget?
	}

	void EditorWidgetManager::onFocusLost(const RenderWindow& window)
	{
		for (auto& widgetData : mActiveWidgets)
		{
			EditorWidgetBase* widget = widgetData.second;
			EditorWidgetContainer* parentContainer = widget->_getParent();
			if (parentContainer == nullptr)
				continue;

			EditorWindowBase* parentWindow = parentContainer->getParentWindow();
			SPtr<RenderWindow> parentRenderWindow = parentWindow->getRenderWindow();

			if (parentRenderWindow.get() != &window)
				continue;

			widget->_setHasFocus(false);
		}
	}

	void EditorWidgetManager::preRegisterWidget(const String& name, std::function<EditorWidgetBase*(EditorWidgetContainer&)> createCallback)
	{
		QueuedCreateCallbacks.push(std::pair<String, std::function<EditorWidgetBase*(EditorWidgetContainer&)>>(name, createCallback));
	}
}