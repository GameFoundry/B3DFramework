//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIDropDownHitBox.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIMouseEvent.h"

using namespace bs;

const String& GUIDropDownHitBox::GetGuiTypeName()
{
	static String name = "DropDownHitBox";
	return name;
}

GUIDropDownHitBox* GUIDropDownHitBox::Create(bool captureMouseOver, bool captureMousePresses)
{
	return new(bs_alloc<GUIDropDownHitBox>())
		GUIDropDownHitBox(captureMouseOver, captureMousePresses, GUIDimensions::Create());
}

GUIDropDownHitBox* GUIDropDownHitBox::Create(bool captureMouseOver, bool captureMousePresses, const GUIOptions& options)
{
	return new(bs_alloc<GUIDropDownHitBox>())
		GUIDropDownHitBox(captureMouseOver, captureMousePresses, GUIDimensions::Create(options));
}

GUIDropDownHitBox::GUIDropDownHitBox(bool captureMouseOver, bool captureMousePresses, const GUIDimensions& dimensions)
	: GUIElementContainer(dimensions), mCaptureMouseOver(captureMouseOver), mCaptureMousePresses(captureMousePresses)
{
	mOptionFlags.Set(GUIElementOption::ClickThrough);
}

void GUIDropDownHitBox::SetBounds(const Rect2I& bounds)
{
	mBounds.clear();
	mBounds.push_back(bounds);

	UpdateClippedBounds();
}

void GUIDropDownHitBox::SetBounds(const Vector<Rect2I>& bounds)
{
	mBounds = bounds;

	UpdateClippedBounds();
}

void GUIDropDownHitBox::UpdateClippedBounds()
{
	mClippedBounds = Rect2I();

	if(mBounds.size() > 0)
	{
		mClippedBounds = mBounds[0];

		for(u32 i = 1; i < (u32)mBounds.size(); i++)
			mClippedBounds.Encapsulate(mBounds[i]);
	}
}

bool GUIDropDownHitBox::CommandEventInternal(const GUICommandEvent& ev)
{
	bool processed = GUIElementContainer::CommandEventInternal(ev);

	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		if(!OnFocusGained.Empty())
			OnFocusGained();

		return false;
	}
	else if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		if(!OnFocusLost.Empty())
			OnFocusLost();

		return false;
	}

	return processed;
}

bool GUIDropDownHitBox::MouseEventInternal(const GUIMouseEvent& ev)
{
	bool processed = GUIElementContainer::MouseEventInternal(ev);

	if(mCaptureMouseOver)
	{
		if(ev.GetType() == GUIMouseEventType::MouseOver)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseOut)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseMove)
		{
			return true;
		}
	}

	if(mCaptureMousePresses)
	{
		if(ev.GetType() == GUIMouseEventType::MouseUp)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseDown)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseDoubleClick)
		{
			return true;
		}
	}

	return processed;
}

bool GUIDropDownHitBox::IsInBoundsInternal(const Vector2I position) const
{
	for(auto& bound : mBounds)
	{
		if(bound.Contains(position))
			return true;
	}

	return false;
}
