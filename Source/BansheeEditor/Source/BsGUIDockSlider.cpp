//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIDockSlider.h"
#include "BsGUIDimensions.h"
#include "BsGUIMouseEvent.h"

namespace bs
{
	const String& GUIDockSlider::getGUITypeName()
	{
		static String name = "DockSlider";
		return name;
	}

	GUIDockSlider::GUIDockSlider(bool horizontal, const String& styleName, const GUIDimensions& dimensions)
		: GUIButtonBase(styleName, GUIContent(HString(L"")), dimensions), mHorizontal(horizontal), mDragInProgress(false)
	{

	}

	GUIDockSlider* GUIDockSlider::create(bool horizontal, const String& styleName)
	{
		return new (bs_alloc<GUIDockSlider>()) GUIDockSlider(horizontal, 
			getStyleName<GUIDockSlider>(styleName), GUIDimensions::create());
	}

	GUIDockSlider* GUIDockSlider::create(bool horizontal, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIDockSlider>()) GUIDockSlider(horizontal, 
			getStyleName<GUIDockSlider>(styleName), GUIDimensions::create(options));
	}

	bool GUIDockSlider::_hasCustomCursor(const Vector2I position, CursorType& type) const
	{
		if (_isInBounds(position) && !_isDisabled())
		{
			type = mHorizontal ? CursorType::SizeNS : CursorType::SizeWE;
			return true;
		}

		return false;
	}

	bool GUIDockSlider::_mouseEvent(const GUIMouseEvent& ev)
	{	
		bool processed = GUIButtonBase::_mouseEvent(ev);

		if(ev.getType() == GUIMouseEventType::MouseDragStart)
		{
			if (!_isDisabled())
			{
				mLastDragPosition = ev.getPosition();
				mDragInProgress = true;
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDrag)
		{
			if (!_isDisabled())
			{
				Vector2I delta = ev.getPosition() - mLastDragPosition;
				mLastDragPosition = ev.getPosition();

				if (!onDragged.empty())
					onDragged(delta);
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDragEnd)
		{
			if (!_isDisabled())
				mDragInProgress = false;

			return true;
		}

		return processed;
	}
}