//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIDropButton.h"
#include "BsGUIDimensions.h"
#include "BsGUIMouseEvent.h"
#include "BsDragAndDropManager.h"

namespace bs
{
	const String& GUIDropButton::getGUITypeName()
	{
		static String name = "DropButton";
		return name;
	}

	GUIDropButton::GUIDropButton(UINT32 dragType, const String& styleName, const GUIDimensions& dimensions)
		:GUIButtonBase(styleName, GUIContent(HString(L"None")), dimensions), mDragType(dragType)
	{

	}

	GUIDropButton::~GUIDropButton()
	{ }

	GUIDropButton* GUIDropButton::create(UINT32 dragType, const String& styleName)
	{
		return new (bs_alloc<GUIDropButton>()) GUIDropButton(dragType, 
			getStyleName<GUIDropButton>(styleName), GUIDimensions::create());
	}

	GUIDropButton* GUIDropButton::create(UINT32 dragType, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIDropButton>()) GUIDropButton(dragType, 
			getStyleName<GUIDropButton>(styleName), GUIDimensions::create(options));
	}

	bool GUIDropButton::_mouseEvent(const GUIMouseEvent& ev)
	{
		bool processed = GUIButtonBase::_mouseEvent(ev);

		if(ev.getType() == GUIMouseEventType::MouseDragAndDropDragged)
		{
			if (!_isDisabled())
			{
				if (DragAndDropManager::instance().isDragInProgress())
				{
					if (DragAndDropManager::instance().getDragTypeId() == mDragType)
					{
						if (!_isOn())
							_setOn(true);
					}
					else
					{
						if (_isOn())
							_setOn(false);
					}
				}
				else
				{
					if (_isOn())
						_setOn(false);
				}
			}

			processed = true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDragAndDropDropped)
		{
			if (!_isDisabled())
			{
				if (_isOn())
					_setOn(false);

				if (DragAndDropManager::instance().isDragInProgress() && DragAndDropManager::instance().getDragTypeId() == mDragType)
				{
					if (!onDataDropped.empty())
						onDataDropped(DragAndDropManager::instance().getDragData());
				}
			}

			processed = true;
		}
		else if (ev.getType() == GUIMouseEventType::MouseDragAndDropLeft)
		{
			if (!_isDisabled())
			{
				if (_isOn())
					_setOn(false);
			}

			processed = true;
		}

		return processed;
	}

	bool GUIDropButton::_acceptDragAndDrop(const Vector2I position, UINT32 typeId) const
	{
		return typeId == mDragType && !_isDisabled();
	}
}