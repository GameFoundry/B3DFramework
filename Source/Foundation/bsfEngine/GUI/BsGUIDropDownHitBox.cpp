//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIDropDownHitBox.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIMouseEvent.h"

namespace bs
{
	const String& GUIDropDownHitBox::GetGUITypeName()
	{
		static String name = "DropDownHitBox";
		return name;
	}

	GUIDropDownHitBox* GUIDropDownHitBox::create(bool captureMouseOver, bool captureMousePresses)
	{
		return new (bs_alloc<GUIDropDownHitBox>())
			GUIDropDownHitBox(captureMouseOver, captureMousePresses, GUIDimensions::create());
	}

	GUIDropDownHitBox* GUIDropDownHitBox::create(bool captureMouseOver, bool captureMousePresses, const GUIOptions& options)
	{
		return new (bs_alloc<GUIDropDownHitBox>())
			GUIDropDownHitBox(captureMouseOver, captureMousePresses, GUIDimensions::create(options));
	}

	GUIDropDownHitBox::GUIDropDownHitBox(bool captureMouseOver,
		bool captureMousePresses, const GUIDimensions& dimensions)
		:GUIElementContainer(dimensions), mCaptureMouseOver(captureMouseOver),
		mCaptureMousePresses(captureMousePresses)
	{
		mOptionFlags.set(GUIElementOption::ClickThrough);
	}

	void GUIDropDownHitBox::SetBounds(const Rect2I& bounds)
	{
		mBounds.clear();
		mBounds.push_back(bounds);

		updateClippedBounds();
	}

	void GUIDropDownHitBox::SetBounds(const Vector<Rect2I>& bounds)
	{
		mBounds = bounds;

		updateClippedBounds();
	}

	void GUIDropDownHitBox::UpdateClippedBounds()
	{
		mClippedBounds = Rect2I();

		if (mBounds.size() > 0)
		{
			mClippedBounds = mBounds[0];

			for (UINT32 i = 1; i < (UINT32)mBounds.size(); i++)
				mClippedBounds.encapsulate(mBounds[i]);
		}
	}

	bool GUIDropDownHitBox::_commandEvent(const GUICommandEvent& ev)
	{
		bool processed = GUIElementContainer::_commandEvent(ev);

		if(ev.getType() == GUICommandEventType::FocusGained)
		{
			if(!onFocusGained.empty())
				onFocusGained();

			return false;
		}
		else If(ev.getType() == GUICommandEventType::FocusLost)
		{
			if(!onFocusLost.empty())
				onFocusLost();

			return false;
		}

		return processed;
	}

	bool GUIDropDownHitBox::_mouseEvent(const GUIMouseEvent& ev)
	{
		bool processed = GUIElementContainer::_mouseEvent(ev);

		if(mCaptureMouseOver)
		{
			if (ev.getType() == GUIMouseEventType::MouseOver)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseOut)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseMove)
			{
				return true;
			}
		}

		if (mCaptureMousePresses)
		{
			if (ev.getType() == GUIMouseEventType::MouseUp)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseDown)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseDoubleClick)
			{
				return true;
			}
		}

		return processed;
	}

	bool GUIDropDownHitBox::_isInBounds(const Vector2I position) const
	{
		for(auto& bound : mBounds)
		{
			if(bound.contains(position))
				return true;
		}

		return false;
	}
};
