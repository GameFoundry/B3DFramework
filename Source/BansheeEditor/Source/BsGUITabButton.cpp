//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUITabButton.h"
#include "BsCGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUIDimensions.h"
#include "BsGUIMouseEvent.h"
#include "BsGUITabbedTitleBar.h"

namespace bs
{
	const UINT32 GUITabButton::DRAG_MIN_DISTANCE = 3;

	const String& GUITabButton::getGUITypeName()
	{
		static String name = "TabButton";
		return name;
	}

	GUITabButton::GUITabButton(const String& styleName, const SPtr<GUIToggleGroup>& toggleGroup, 
		UINT32 index, const GUIContent& content, const GUIDimensions& dimensions)
		:GUIToggle(styleName, content, toggleGroup, dimensions), mIndex(index), mDraggedState(false)
	{

	}

	GUITabButton* GUITabButton::create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, 
		const HString& text, const String& styleName)
	{
		return new (bs_alloc<GUITabButton>()) GUITabButton(
			getStyleName<GUITabButton>(styleName), toggleGroup, index, GUIContent(text), GUIDimensions::create());
	}

	GUITabButton* GUITabButton::create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, 
		const HString& text, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUITabButton>()) GUITabButton(
			getStyleName<GUITabButton>(styleName), toggleGroup, index, GUIContent(text), GUIDimensions::create(options));
	}

	GUITabButton* GUITabButton::create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, 
		const GUIContent& content, const String& styleName)
	{
		return new (bs_alloc<GUITabButton>()) GUITabButton(
			getStyleName<GUITabButton>(styleName), toggleGroup, index, content, GUIDimensions::create());
	}

	GUITabButton* GUITabButton::create(const SPtr<GUIToggleGroup>& toggleGroup, UINT32 index, 
		const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUITabButton>()) GUITabButton(
			getStyleName<GUITabButton>(styleName), toggleGroup, index, content, GUIDimensions::create(options));
	}

	void GUITabButton::toggleOn()
	{
		_setElementDepth(0);

		GUIToggle::toggleOn();
	}

	void GUITabButton::toggleOff()
	{
		_setElementDepth(2);

		GUIToggle::toggleOff();
	}

	void GUITabButton::_setDraggedState(bool active) 
	{
		if(mDraggedState == active)
			return;

		mDraggedState = active; 

		if(mDraggedState)
		{
			mInactiveState = getState();

			if(mInactiveState != GUIElementState::Normal)
				_setState(GUIElementState::Normal);
		}
		else
		{
			if(getState() != mInactiveState)
				_setState(mInactiveState);
		}
	}

	bool GUITabButton::_mouseEvent(const GUIMouseEvent& ev)
	{	
		if(ev.getType() == GUIMouseEventType::MouseOver)
		{
			if (!_isDisabled())
			{
				GUIElementState state = _isOn() ? GUIElementState::HoverOn : GUIElementState::Hover;

				if (!mDraggedState)
				{
					_setState(state);

					if (!onHover.empty())
						onHover();
				}
				else
					mInactiveState = state;
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseOut)
		{
			if (!_isDisabled())
			{
				GUIElementState state = _isOn() ? GUIElementState::NormalOn : GUIElementState::Normal;

				if (!mDraggedState)
				{
					_setState(state);

					if (!onOut.empty())
						onOut();
				}
				else
					mInactiveState = state;
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDown)
		{
			if (!_isDisabled())
			{
				if (!mDraggedState)
					_setState(_isOn() ? GUIElementState::ActiveOn : GUIElementState::Active);

				_setElementDepth(0);
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseUp)
		{
			if (!_isDisabled())
			{
				if (!mDraggedState)
				{
					_setState(_isOn() ? GUIElementState::HoverOn : GUIElementState::Hover);

					if (!onClick.empty())
						onClick();

					if (!mIsToggled)
						toggleOn();
				}
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDragStart)
		{
			if (!_isDisabled())
				mDragStartPosition = ev.getPosition();

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDrag)
		{
			if (!_isDisabled())
			{
				UINT32 dist = mDragStartPosition.manhattanDist(ev.getPosition());

				if (dist > DRAG_MIN_DISTANCE)
				{
					if (!onDragged.empty())
						onDragged(mIndex, ev.getPosition());
				}
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDragEnd)
		{
			if (!_isDisabled())
			{
				UINT32 dist = mDragStartPosition.manhattanDist(ev.getPosition());

				if (dist > DRAG_MIN_DISTANCE)
				{
					if (!onDragEnd.empty())
						onDragEnd(mIndex, ev.getPosition());
				}
			}

			return true;
		}

		return false;
	}
}