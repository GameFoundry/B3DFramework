//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorWidgetContainer.h"
#include "BsGUITabbedTitleBar.h"
#include "BsEditorWidget.h"
#include "BsDragAndDropManager.h"
#include "BsEditorWindow.h"
#include "BsGUIPanel.h"
#include "BsGUIWidget.h"
#include "BsMath.h"
#include "BsGUILayoutX.h"
#include "BsCursor.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const UINT32 EditorWidgetContainer::TitleBarHeight = 16;

	EditorWidgetContainer::EditorWidgetContainer(GUIWidget* parent, EditorWindowBase* parentEditorWindow)
		: mParentWindow(parentEditorWindow), mTitleBar(nullptr), mTitleBarPanel(nullptr), mParent(parent), mX(0), mY(0)
		, mWidth(0), mHeight(0), mActiveWidget(-1)
	{
		mTitleBarPanel = parent->getPanel()->addNewElement<GUIPanel>();

		mTitleBar = GUITabbedTitleBar::create();
		mTitleBar->onTabActivated.connect(std::bind(&EditorWidgetContainer::tabActivated, this, _1));
		mTitleBar->onTabClosed.connect(std::bind(&EditorWidgetContainer::tabClosed, this, _1));
		mTitleBar->onTabDraggedOff.connect(std::bind(&EditorWidgetContainer::tabDraggedOff, this, _1));
		mTitleBar->onTabDraggedOn.connect(std::bind(&EditorWidgetContainer::tabDraggedOn, this, _1));
		mTitleBar->onTabMaximized.connect(std::bind(&EditorWidgetContainer::tabMaximized, this, _1));

		GUILayout* titleBarLayout = mTitleBarPanel->addNewElement<GUILayoutX>();
		titleBarLayout->addElement(mTitleBar);
	}

	EditorWidgetContainer::~EditorWidgetContainer()
	{
		while (mWidgets.size() > 0)
			mWidgets.begin()->second->close();

		GUILayout::destroy(mTitleBarPanel);
		GUIElement::destroy(mTitleBar);
	}

	void EditorWidgetContainer::update()
	{
		for (auto& widget : mWidgets)
		{
			widget.second->update();
		}
	}

	void EditorWidgetContainer::refreshWidgetNames()
	{
		for (auto& curWidget : mWidgets)
		{
			INT32 tabIdx = curWidget.first;
			mTitleBar->updateTabName((UINT32)tabIdx, curWidget.second->getDisplayName());
		}
	}

	void EditorWidgetContainer::add(EditorWidgetBase& widget)
	{
		insert((UINT32)mWidgets.size(), widget);
	}

	void EditorWidgetContainer::remove(EditorWidgetBase& widget)
	{
		removeInternal(widget);

		if(!onWidgetClosed.empty())
			onWidgetClosed();
	}

	void EditorWidgetContainer::removeInternal(EditorWidgetBase& widget)
	{
		INT32 tabIdx = 0;
		for(auto& curWidget : mWidgets)
		{
			if(curWidget.second == &widget)
			{
				tabIdx = curWidget.first;
				break;
			}
		}

		if(tabIdx == -1)
			return;

		mWidgets.erase((UINT32)tabIdx);
		mTitleBar->removeTab((UINT32)tabIdx);
		widget._changeParent(nullptr, tabIdx);

		if(tabIdx == mActiveWidget)
		{
			mActiveWidget = -1;
			if(mTitleBar->getNumTabs() > 0)
			{
				setActiveWidget(mTitleBar->getTabIdx(0));
			}
		}
	}

	void EditorWidgetContainer::insert(UINT32 idx, EditorWidgetBase& widget)
	{
		if(contains(widget))
			return;

		idx = Math::clamp(idx, 0U, (UINT32)mWidgets.size());

		UINT32 tabIdx = mTitleBar->insertTab(idx, widget.getDisplayName());
		mWidgets[tabIdx] = &widget;
		widget._changeParent(this, idx);

		if(mActiveWidget == -1)
			setActiveWidget(mTitleBar->getTabIdx(mTitleBar->getNumTabs() - 1));
		else
			widget._disable();

		onWidgetAdded();
	}

	bool EditorWidgetContainer::contains(EditorWidgetBase& widget)
	{
		for(auto& curWidget : mWidgets)
		{
			if(curWidget.second == &widget)
				return true;
		}

		return false;
	}

	EditorWidgetBase* EditorWidgetContainer::getWidget(UINT32 idx) const
	{
		if(idx >= (UINT32)mWidgets.size())
			return nullptr;

		UINT32 tabIdx = mTitleBar->getTabIdx(idx);

		auto iterFind = mWidgets.find(tabIdx);

		if(iterFind != mWidgets.end())
			return iterFind->second;

		return nullptr;
	}

	EditorWidgetBase* EditorWidgetContainer::getActiveWidget() const
	{
		if (mActiveWidget >= 0)
		{
			auto iterFind = mWidgets.find(mActiveWidget);
			
			if (iterFind != mWidgets.end())
				return iterFind->second;
		}

		return nullptr;
	}

	void EditorWidgetContainer::setSize(UINT32 width, UINT32 height)
	{
		// TODO - Title bar is always TitleBarHeight size, so what happens when the container area is smaller than that?
		mTitleBarPanel->setWidth(width);
		mTitleBarPanel->setHeight(TitleBarHeight);

		if(mActiveWidget >= 0)
		{
			EditorWidgetBase* activeWidgetPtr = mWidgets[mActiveWidget];

			Vector2I widgetSize = windowToWidgetSize(Vector2I(width, height));
			activeWidgetPtr->_setSize((UINT32)widgetSize.x, (UINT32)widgetSize.y);
		}

		mWidth = width;
		mHeight = height;
	}

	void EditorWidgetContainer::setPosition(INT32 x, INT32 y)
	{
		mTitleBarPanel->setPosition(x, y);

		if(mActiveWidget >= 0)
		{
			EditorWidgetBase* activeWidgetPtr = mWidgets[mActiveWidget];

			activeWidgetPtr->_setPosition(x, y + TitleBarHeight);
		}

		mX = x;
		mY = y;
	}

	void EditorWidgetContainer::setActiveWidget(UINT32 idx)
	{
		if(mActiveWidget == idx)
			return;

		mActiveWidget = idx;

		for(auto& curWidget : mWidgets)
		{
			if(curWidget.first != (UINT32)mActiveWidget)
				curWidget.second->_disable();
			else
				curWidget.second->_enable();
		}

		mTitleBar->setActive(idx);

		setPosition(mX, mY);
		setSize(mWidth, mHeight);
	}

	void EditorWidgetContainer::tabActivated(UINT32 uniqueIdx)
	{
		setActiveWidget(uniqueIdx);
	}

	void EditorWidgetContainer::tabClosed(UINT32 uniqueIdx)
	{
		EditorWidgetBase* widget = mWidgets[uniqueIdx];
		widget->close();
	}

	void EditorWidgetContainer::tabMaximized(UINT32 uniqueIdx)
	{
		onMaximized();
	}

	void EditorWidgetContainer::tabDraggedOff(UINT32 uniqueIdx)
	{
		EditorWidgetBase* widget = mWidgets[uniqueIdx];
		removeInternal(*widget);

		DragAndDropManager::instance().startDrag((UINT32)DragAndDropType::EditorWidget, (void*)widget, 
			std::bind(&EditorWidgetContainer::tabDroppedCallback, _1), false);

		if(!onWidgetClosed.empty())
			onWidgetClosed();
	}

	void EditorWidgetContainer::tabDraggedOn(UINT32 seqIdx)
	{
#if BS_DEBUG_MODE
		if(!DragAndDropManager::instance().isDragInProgress())
			BS_EXCEPT(InternalErrorException, "Tab drag and drop reported but no drag in progress.");

		if(DragAndDropManager::instance().getDragTypeId() != (UINT32)DragAndDropType::EditorWidget)
			BS_EXCEPT(InternalErrorException, "Tab drag and drop reported but drag type is invalid.");
#endif

		EditorWidgetBase* draggedWidget = static_cast<EditorWidgetBase*>(DragAndDropManager::instance().getDragData());

		insert(seqIdx, *draggedWidget);
		setActiveWidget(mTitleBar->getTabIdx(seqIdx));
	}

	void EditorWidgetContainer::tabDroppedCallback(bool wasDragProcessed)
	{
		if(DragAndDropManager::instance().getDragTypeId() != (UINT32)DragAndDropType::EditorWidget)
			return;

		if(!wasDragProcessed)
		{
			EditorWidgetBase* draggedWidget = static_cast<EditorWidgetBase*>(DragAndDropManager::instance().getDragData());
			EditorWindow* newWindow = EditorWindow::create();

			newWindow->widgets().add(*draggedWidget);

			Vector2I mousePos = Cursor::instance().getScreenPosition();
			newWindow->setPosition(mousePos.x, mousePos.y);
		}
	}

	Rect2I EditorWidgetContainer::getContentBounds() const
	{
		return Rect2I(mX, mY + TitleBarHeight, mWidth, (UINT32)std::max(0, (INT32)mHeight - (INT32)TitleBarHeight));
	}

	Vector<Rect2I> EditorWidgetContainer::getDraggableAreas() const
	{
		return mTitleBar->calcDraggableAreas(mX, mY, mWidth, TitleBarHeight);
	}

	void EditorWidgetContainer::_notifyWidgetDestroyed(EditorWidgetBase* widget)
	{
		for(auto& curWidget : mWidgets)
		{
			if(curWidget.second == widget)
			{
				removeInternal(*widget);

				if(!onWidgetClosed.empty())
					onWidgetClosed();

				return;
			}
		}		
	}

	Vector2I EditorWidgetContainer::windowToWidgetSize(const Vector2I& windowSize)
	{
		Vector2I widgetSize = windowSize;
		widgetSize.y = std::max(0, widgetSize.y - (INT32)TitleBarHeight);

		return widgetSize;
	}

	Vector2I EditorWidgetContainer::widgetToWindowSize(const Vector2I& widgetSize)
	{
		Vector2I windowSize = widgetSize;
		windowSize.y += TitleBarHeight;

		return windowSize;
	}
}